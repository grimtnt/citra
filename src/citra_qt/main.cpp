// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <clocale>
#include <memory>
#include <thread>
#include <glad/glad.h>
#define QT_NO_OPENGL
#include <QDesktopWidget>
#include <QFileDialog>
#include <QFutureWatcher>
#include <QMessageBox>
#include <QtConcurrent/QtConcurrentRun>
#include <QtGui>
#include <QtWidgets>
#ifdef ENABLE_DISCORD_RPC
#include <discord_rpc.h>
#endif
#include "citra_qt/aboutdialog.h"
#include "citra_qt/bootmanager.h"
#include "citra_qt/camera/qt_multimedia_camera.h"
#include "citra_qt/camera/still_image_camera.h"
#include "citra_qt/cheat_gui.h"
#include "citra_qt/cheatsearch.h"
#include "citra_qt/configuration/config.h"
#include "citra_qt/configuration/configure_dialog.h"
#include "citra_qt/control_panel.h"
#include "citra_qt/game_list.h"
#include "citra_qt/hotkeys.h"
#include "citra_qt/main.h"
#include "citra_qt/multiplayer/state.h"
#include "citra_qt/swkbd.h"
#include "citra_qt/ui_settings.h"
#include "citra_qt/util/clickable_label.h"
#include "citra_qt/util/console.h"
#include "common/common_paths.h"
#include "common/logging/backend.h"
#include "common/logging/filter.h"
#include "common/logging/log.h"
#include "common/logging/text_formatter.h"
#include "common/scm_rev.h"
#include "common/scope_exit.h"
#include "common/string_util.h"
#include "core/core.h"
#include "core/file_sys/archive_source_sd_savedata.h"
#include "core/hle/service/fs/archive.h"
#include "core/hle/service/ptm/ptm.h"
#include "core/loader/loader.h"
#include "core/movie.h"
#include "core/settings.h"

#ifdef QT_STATICPLUGIN
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
#endif

#ifdef _WIN32
extern "C" {
// tells Nvidia drivers to use the dedicated GPU by default on laptops with switchable graphics
__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}
#endif

/**
 * "Callouts" are one-time instructional messages shown to the user. In the config settings, there
 * is a bitfield "callout_flags" options, used to track if a message has already been shown to the
 * user. This is 32-bits - if we have more than 32 callouts, we should retire and recyle old ones.
 */
enum class CalloutFlag : uint32_t {};

#ifdef ENABLE_DISCORD_RPC
s64 g_start_time = 0;

static void HandleDiscordDisconnected(int errorCode, const char* message) {
    LOG_ERROR(Frontend, "Disconnected, error: {} ({})", message, errorCode);
}

static void HandleDiscordError(int errorCode, const char* message) {
    LOG_ERROR(Frontend, "Error: {} ({})", message, errorCode);
}
#endif

static void ShowCalloutMessage(const QString& message, CalloutFlag flag) {
    if (UISettings::values.callout_flags & static_cast<uint32_t>(flag)) {
        return;
    }

    UISettings::values.callout_flags |= static_cast<uint32_t>(flag);

    QMessageBox msg;
    msg.setText(message);
    msg.setStandardButtons(QMessageBox::Ok);
    msg.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    msg.setStyleSheet("QLabel{min-width: 900px;}");
    msg.exec();
}

void GMainWindow::ShowCallouts() {}

GMainWindow::GMainWindow() : config(new Config()), emu_thread(nullptr) {
    // register types to use in slots and signals
    qRegisterMetaType<size_t>("size_t");
    qRegisterMetaType<Service::AM::InstallStatus>("Service::AM::InstallStatus");

    LoadTranslation();

    setAcceptDrops(true);
    ui.setupUi(this);
    statusBar()->hide();

    default_theme_paths = QIcon::themeSearchPaths();
    UpdateUITheme();

    Network::Init();

    InitializeWidgets();
    InitializeRecentFileMenuActions();
    InitializeHotkeys();

    SetDefaultUIGeometry();
    RestoreUIState();

    ConnectMenuEvents();
    ConnectWidgetEvents();

    SetupUIStrings();

    game_list->LoadCompatibilityList();
    game_list->PopulateAsync(UISettings::values.game_dirs);

    // Show one-time "callout" messages to the user
    ShowCallouts();

    QStringList args = QApplication::arguments();
    if (args.length() >= 2) {
        BootGame(args[1]);
    }
}

GMainWindow::~GMainWindow() {
    // will get automatically deleted otherwise
    if (render_window->parent() == nullptr)
        delete render_window;
    Network::Shutdown();
}

void GMainWindow::InitializeWidgets() {
    render_window = new GRenderWindow(this, emu_thread.get());
    render_window->hide();

    game_list = new GameList(this);
    ui.horizontalLayout->addWidget(game_list);

    game_list_placeholder = new GameListPlaceholder(this);
    ui.horizontalLayout->addWidget(game_list_placeholder);
    game_list_placeholder->setVisible(false);

    multiplayer_state = new MultiplayerState(this, game_list->GetModel(), ui.action_Leave_Room,
                                             ui.action_Show_Room);
    multiplayer_state->setVisible(false);

    // Create status bar
    message_label = new QLabel();
    // Configured separately for left alignment
    message_label->setVisible(false);
    message_label->setFrameStyle(QFrame::NoFrame);
    message_label->setContentsMargins(4, 0, 4, 0);
    message_label->setAlignment(Qt::AlignLeft);
    statusBar()->addPermanentWidget(message_label, 1);

    progress_bar = new QProgressBar();
    progress_bar->setMaximum(INT_MAX);
    progress_bar->hide();
    statusBar()->addPermanentWidget(progress_bar);

    emu_speed_label = new QLabel();
    emu_speed_label->setToolTip(tr("Current emulation speed. Values higher or lower than 100% "
                                   "indicate emulation is running faster or slower than a 3DS."));
    game_fps_label = new QLabel();
    game_fps_label->setToolTip(tr("How many frames per second the game is currently displaying. "
                                  "This will vary from game to game and scene to scene."));
    emu_frametime_label = new QLabel();
    emu_frametime_label->setToolTip(
        tr("Time taken to emulate a 3DS frame, not counting framelimiting or v-sync. For "
           "full-speed emulation this should be at most 16.67 ms."));

    for (auto& label : {emu_speed_label, game_fps_label, emu_frametime_label}) {
        label->setVisible(false);
        label->setFrameStyle(QFrame::NoFrame);
        label->setContentsMargins(4, 0, 4, 0);
        statusBar()->addPermanentWidget(label, 0);
    }
    statusBar()->addPermanentWidget(multiplayer_state->GetStatusText(), 0);
    statusBar()->addPermanentWidget(multiplayer_state->GetStatusIcon(), 0);
    statusBar()->setVisible(true);

    // Removes an ugly inner border from the status bar widgets under Linux
    setStyleSheet("QStatusBar::item{border: none;}");

    QActionGroup* actionGroup_ScreenLayouts = new QActionGroup(this);
    actionGroup_ScreenLayouts->addAction(ui.action_Screen_Layout_Default);
    actionGroup_ScreenLayouts->addAction(ui.action_Screen_Layout_Single_Screen);
    actionGroup_ScreenLayouts->addAction(ui.action_Screen_Layout_Medium_Screen);
    actionGroup_ScreenLayouts->addAction(ui.action_Screen_Layout_Large_Screen);
    actionGroup_ScreenLayouts->addAction(ui.action_Screen_Layout_Side_by_Side);
}

void GMainWindow::InitializeRecentFileMenuActions() {
    for (int i = 0; i < max_recent_files_item; ++i) {
        actions_recent_files[i] = new QAction(this);
        actions_recent_files[i]->setVisible(false);
        connect(actions_recent_files[i], &QAction::triggered, this, &GMainWindow::OnMenuRecentFile);

        ui.menu_recent_files->addAction(actions_recent_files[i]);
    }

    UpdateRecentFiles();
}

void GMainWindow::InitializeHotkeys() {
    RegisterHotkey("Main Window", "Load File", QKeySequence::Open);
    RegisterHotkey("Main Window", "Start Emulation");
    RegisterHotkey("Main Window", "Continue/Pause", QKeySequence(Qt::Key_F4));
    RegisterHotkey("Main Window", "Restart", QKeySequence(Qt::Key_F5));
    RegisterHotkey("Main Window", "Swap Screens", QKeySequence("F9"));
    RegisterHotkey("Main Window", "Toggle Screen Layout", QKeySequence("F10"));
    RegisterHotkey("Main Window", "Fullscreen", QKeySequence::FullScreen);
    RegisterHotkey("Main Window", "Exit Fullscreen", QKeySequence(Qt::Key_Escape),
                   Qt::ApplicationShortcut);
    RegisterHotkey("Main Window", "Toggle Speed Limit", QKeySequence("CTRL+Z"),
                   Qt::ApplicationShortcut);
    RegisterHotkey("Main Window", "Increase Speed Limit", QKeySequence("+"),
                   Qt::ApplicationShortcut);
    RegisterHotkey("Main Window", "Decrease Speed Limit", QKeySequence("-"),
                   Qt::ApplicationShortcut);
    LoadHotkeys();

    connect(GetHotkey("Main Window", "Load File", this), &QShortcut::activated, this,
            &GMainWindow::OnMenuLoadFile);
    connect(GetHotkey("Main Window", "Start Emulation", this), &QShortcut::activated, this,
            &GMainWindow::OnStartGame);
    connect(GetHotkey("Main Window", "Continue/Pause", this), &QShortcut::activated, this, [&] {
        if (emulation_running) {
            if (emu_thread->IsRunning()) {
                OnPauseGame();
            } else {
                OnStartGame();
            }
        }
    });
    connect(GetHotkey("Main Window", "Restart", this), &QShortcut::activated, this, [&] {
        if (!Core::System::GetInstance().IsPoweredOn())
            return;
        BootGame(QString(UISettings::values.recent_files.first()));
    });
    connect(GetHotkey("Main Window", "Swap Screens", render_window), &QShortcut::activated,
            ui.action_Screen_Layout_Swap_Screens, &QAction::trigger);
    connect(GetHotkey("Main Window", "Toggle Screen Layout", render_window), &QShortcut::activated,
            this, &GMainWindow::ToggleScreenLayout);
    connect(GetHotkey("Main Window", "Fullscreen", render_window), &QShortcut::activated,
            ui.action_Fullscreen, &QAction::trigger);
    connect(GetHotkey("Main Window", "Fullscreen", render_window), &QShortcut::activatedAmbiguously,
            ui.action_Fullscreen, &QAction::trigger);
    connect(GetHotkey("Main Window", "Exit Fullscreen", this), &QShortcut::activated, this, [&] {
        if (emulation_running) {
            ui.action_Fullscreen->setChecked(false);
            ToggleFullscreen();
        }
    });
    connect(GetHotkey("Main Window", "Toggle Speed Limit", this), &QShortcut::activated, this, [&] {
        Settings::values.use_frame_limit = !Settings::values.use_frame_limit;
        UpdateStatusBar();
    });
    constexpr u16 SPEED_LIMIT_STEP = 5;
    connect(GetHotkey("Main Window", "Increase Speed Limit", this), &QShortcut::activated, this,
            [&] {
                if (Settings::values.frame_limit < 9999 - SPEED_LIMIT_STEP) {
                    Settings::values.frame_limit += SPEED_LIMIT_STEP;
                    UpdateStatusBar();
                }
            });
    connect(GetHotkey("Main Window", "Decrease Speed Limit", this), &QShortcut::activated, this,
            [&] {
                if (Settings::values.frame_limit > SPEED_LIMIT_STEP) {
                    Settings::values.frame_limit -= SPEED_LIMIT_STEP;
                    UpdateStatusBar();
                }
            });
}

void GMainWindow::SetDefaultUIGeometry() {
    // geometry: 55% of the window contents are in the upper screen half, 45% in the lower half
    const QRect screenRect = QApplication::desktop()->screenGeometry(this);

    const int w = screenRect.width() * 2 / 3;
    const int h = screenRect.height() / 2;
    const int x = (screenRect.x() + screenRect.width()) / 2 - w / 2;
    const int y = (screenRect.y() + screenRect.height()) / 2 - h * 55 / 100;

    setGeometry(x, y, w, h);
}

void GMainWindow::RestoreUIState() {
    restoreGeometry(UISettings::values.geometry);
    restoreState(UISettings::values.state);
    render_window->restoreGeometry(UISettings::values.renderwindow_geometry);

    ui.action_Cheats->setEnabled(false);
    ui.action_Cheat_Search->setEnabled(false);
    ui.action_Set_Play_Coins->setEnabled(false);
    ui.action_Play->setEnabled(false);
    ui.action_Record->setEnabled(false);

    game_list->LoadInterfaceLayout();

    ui.action_Single_Window_Mode->setChecked(UISettings::values.single_window_mode);
    ToggleWindowMode();

    ui.action_Fullscreen->setChecked(UISettings::values.fullscreen);
    SyncMenuUISettings();

    ui.action_Display_Dock_Widget_Headers->setChecked(UISettings::values.display_titlebar);
    OnDisplayTitleBars(ui.action_Display_Dock_Widget_Headers->isChecked());

    ui.action_Show_Filter_Bar->setChecked(UISettings::values.show_filter_bar);
    game_list->setFilterVisible(ui.action_Show_Filter_Bar->isChecked());

    ui.action_Show_Status_Bar->setChecked(UISettings::values.show_status_bar);
    statusBar()->setVisible(ui.action_Show_Status_Bar->isChecked());
    Util::ToggleConsole();
}

void GMainWindow::ConnectWidgetEvents() {
    connect(game_list, &GameList::GameChosen, this, &GMainWindow::OnGameListLoadFile);
    connect(game_list, &GameList::OpenDirectory, this, &GMainWindow::OnGameListOpenDirectory);
    connect(game_list, &GameList::OpenFolderRequested, this, &GMainWindow::OnGameListOpenFolder);
    connect(game_list, &GameList::AddDirectory, this, &GMainWindow::OnGameListAddDirectory);
    connect(game_list_placeholder, &GameListPlaceholder::AddDirectory, this,
            &GMainWindow::OnGameListAddDirectory);
    connect(game_list, &GameList::ShowList, this, &GMainWindow::OnGameListShowList);

    connect(this, &GMainWindow::EmulationStarting, render_window,
            &GRenderWindow::OnEmulationStarting);
    connect(this, &GMainWindow::EmulationStopping, render_window,
            &GRenderWindow::OnEmulationStopping);

    connect(&status_bar_update_timer, &QTimer::timeout, this, &GMainWindow::UpdateStatusBar);
    connect(&movie_play_timer, &QTimer::timeout, this, [&] {
        bool playing = Core::Movie::GetInstance().IsPlayingInput();

        ui.action_Play->setText(playing ? tr("Stop Playing") : tr("Play"));
        ui.action_Record->setEnabled(!playing);

        if (!playing)
            movie_play_timer.stop();
    });

    connect(this, &GMainWindow::UpdateProgress, this, &GMainWindow::OnUpdateProgress);
    connect(this, &GMainWindow::CIAInstallReport, this, &GMainWindow::OnCIAInstallReport);
    connect(this, &GMainWindow::CIAInstallFinished, this, &GMainWindow::OnCIAInstallFinished);
}

void GMainWindow::ConnectMenuEvents() {
    // File
    connect(ui.action_Load_File, &QAction::triggered, this, &GMainWindow::OnMenuLoadFile);
    connect(ui.action_Install_CIA, &QAction::triggered, this, &GMainWindow::OnMenuInstallCIA);
    connect(ui.action_Exit, &QAction::triggered, this, &QMainWindow::close);

    // Emulation
    connect(ui.action_Start, &QAction::triggered, this, &GMainWindow::OnStartGame);
    connect(ui.action_Pause, &QAction::triggered, this, &GMainWindow::OnPauseGame);
    connect(ui.action_Stop, &QAction::triggered, this, &GMainWindow::OnStopGame);
    connect(ui.action_Restart, &QAction::triggered, this,
            [&] { BootGame(QString(UISettings::values.recent_files.first())); });
    connect(ui.action_Configure, &QAction::triggered, this, &GMainWindow::OnConfigure);
    connect(ui.action_Cheats, &QAction::triggered, this, &GMainWindow::OnCheats);
    connect(ui.action_Cheat_Search, &QAction::triggered, this, &GMainWindow::OnCheatSearch);
    connect(ui.action_Control_Panel, &QAction::triggered, this, &GMainWindow::OnControlPanel);

    // View
    connect(ui.action_Single_Window_Mode, &QAction::triggered, this,
            &GMainWindow::ToggleWindowMode);
    connect(ui.action_Display_Dock_Widget_Headers, &QAction::triggered, this,
            &GMainWindow::OnDisplayTitleBars);
    ui.action_Show_Filter_Bar->setShortcut(tr("CTRL+F"));
    connect(ui.action_Show_Filter_Bar, &QAction::triggered, this, &GMainWindow::OnToggleFilterBar);
    connect(ui.action_Show_Status_Bar, &QAction::triggered, statusBar(), &QStatusBar::setVisible);
    ui.action_Fullscreen->setShortcut(GetHotkey("Main Window", "Fullscreen", this)->key());
    ui.action_Screen_Layout_Swap_Screens->setShortcut(
        GetHotkey("Main Window", "Swap Screens", this)->key());
    ui.action_Screen_Layout_Swap_Screens->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(ui.action_Fullscreen, &QAction::triggered, this, &GMainWindow::ToggleFullscreen);

    connect(ui.action_Screen_Layout_Default, &QAction::triggered, this,
            &GMainWindow::ChangeScreenLayout);
    connect(ui.action_Screen_Layout_Single_Screen, &QAction::triggered, this,
            &GMainWindow::ChangeScreenLayout);
    connect(ui.action_Screen_Layout_Medium_Screen, &QAction::triggered, this,
            &GMainWindow::ChangeScreenLayout);
    connect(ui.action_Screen_Layout_Large_Screen, &QAction::triggered, this,
            &GMainWindow::ChangeScreenLayout);
    connect(ui.action_Screen_Layout_Side_by_Side, &QAction::triggered, this,
            &GMainWindow::ChangeScreenLayout);
    connect(ui.action_Screen_Layout_Swap_Screens, &QAction::triggered, this,
            &GMainWindow::OnSwapScreens);

    // Misc
    connect(ui.action_Set_Play_Coins, &QAction::triggered, this, &GMainWindow::OnSetPlayCoins);

    // Movie
    connect(ui.action_Record, &QAction::triggered, this, &GMainWindow::OnRecordMovie);
    connect(ui.action_Play, &QAction::triggered, this, &GMainWindow::OnPlayMovie);

    // Multiplayer
    connect(ui.action_View_Lobby, &QAction::triggered, multiplayer_state,
            &MultiplayerState::OnViewLobby);
    connect(ui.action_Start_Room, &QAction::triggered, multiplayer_state,
            &MultiplayerState::OnCreateRoom);
    connect(ui.action_Leave_Room, &QAction::triggered, multiplayer_state,
            &MultiplayerState::OnCloseRoom);
    connect(ui.action_Connect_To_Room, &QAction::triggered, multiplayer_state,
            &MultiplayerState::OnDirectConnectToRoom);
    connect(ui.action_Show_Room, &QAction::triggered, multiplayer_state,
            &MultiplayerState::OnOpenNetworkRoom);

    // Help
    connect(ui.action_About, &QAction::triggered, this, &GMainWindow::OnMenuAboutCitra);
}

void GMainWindow::OnDisplayTitleBars(bool show) {
    QList<QDockWidget*> widgets = findChildren<QDockWidget*>();

    if (show) {
        for (QDockWidget* widget : widgets) {
            QWidget* old = widget->titleBarWidget();
            widget->setTitleBarWidget(nullptr);
            if (old != nullptr)
                delete old;
        }
    } else {
        for (QDockWidget* widget : widgets) {
            QWidget* old = widget->titleBarWidget();
            widget->setTitleBarWidget(new QWidget());
            if (old != nullptr)
                delete old;
        }
    }
}

bool GMainWindow::LoadROM(const QString& filename) {
    // Shutdown previous session if the emu thread is still active...
    if (emu_thread != nullptr)
        ShutdownGame();

    render_window->InitRenderTarget();
    render_window->MakeCurrent();

    if (!gladLoadGL()) {
        QMessageBox::critical(this, tr("Error while initializing OpenGL 3.3 Core!"),
                              tr("Your GPU may not support OpenGL 3.3, or you do not "
                                 "have the latest graphics driver."));
        return false;
    }

    Core::System& system{Core::System::GetInstance()};

    const Core::System::ResultStatus result{system.Load(render_window, filename.toStdString())};

    if (result != Core::System::ResultStatus::Success) {
        switch (result) {
        case Core::System::ResultStatus::ErrorGetLoader:
            LOG_CRITICAL(Frontend, "Failed to obtain loader for {}!", filename.toStdString());
            QMessageBox::critical(this, tr("Error while loading ROM!"),
                                  tr("The ROM format is not supported."));
            break;

        case Core::System::ResultStatus::ErrorSystemMode:
            LOG_CRITICAL(Frontend, "Failed to load ROM!");
            QMessageBox::critical(this, tr("Error while loading ROM!"),
                                  tr("Could not determine the system mode."));
            break;

        case Core::System::ResultStatus::ErrorLoader_ErrorEncrypted: {
            QMessageBox::critical(
                this, tr("Error while loading ROM!"),
                tr("The game that you are trying to load must be decrypted before being used with "
                   "Citra. A real 3DS is required.<br/><br/>"
                   "For more information on dumping and decrypting games, please see the following "
                   "wiki pages: <ul>"
                   "<li><a "
                   "href='https://github.com/valentinvanelslande/citra/wiki/"
                   "Dumping-Game-Cartridges'>Dumping Game "
                   "Cartridges</a></li>"
                   "<li><a "
                   "href='https://github.com/valentinvanelslande/citra/wiki/"
                   "Dumping-Installed-Titles'>Dumping "
                   "Installed Titles</a></li>"
                   "</ul>"));
            break;
        }
        case Core::System::ResultStatus::ErrorLoader_ErrorInvalidFormat:
            QMessageBox::critical(this, tr("Error while loading ROM!"),
                                  tr("The ROM format is not supported."));
            break;

        case Core::System::ResultStatus::ErrorVideoCore:
            QMessageBox::critical(
                this, tr("An error occured in the video core."),
                tr("Citra has encountered an error while running the video core, please see the "
                   "log for more details."
                   "To access the log, Click Open Log Location in the general tab of the "
                   "configuration window.<br/><br/>"
                   "Ensure that you have the latest graphics drivers for your GPU."));
            break;

        default:
            QMessageBox::critical(
                this, tr("Error while loading ROM!"),
                tr("An unknown error occured. Please see the log for more details."));
            break;
        }
        return false;
    }

    std::string title;
    system.GetAppLoader().ReadTitle(title);
    game_title = QString::fromStdString(title);
    SetupUIStrings();
#ifdef ENABLE_DISCORD_RPC
    g_start_time = time(NULL);
    DiscordEventHandlers handlers{};
    handlers.disconnected = HandleDiscordDisconnected;
    handlers.errored = HandleDiscordError;
    Discord_Initialize("451776535058448385", &handlers, 0, NULL);
    DiscordRichPresence presence{};
    presence.state = game_title.isEmpty() ? "Unknown game" : game_title.toLocal8Bit().constData();
    presence.details = "Playing";
    presence.startTimestamp = g_start_time;
    presence.largeImageKey = "icon";
    Discord_UpdatePresence(&presence);
#endif

    return true;
}

void GMainWindow::BootGame(const QString& filename) {
    LOG_INFO(Frontend, "Citra starting...");
    StoreRecentFile(filename); // Put the filename on top of the list

    if (!LoadROM(filename))
        return;

    // Create and start the emulation thread
    emu_thread = std::make_unique<EmuThread>(render_window);
    emit EmulationStarting(emu_thread.get());
    render_window->moveContext();
    emu_thread->start();

    connect(render_window, &GRenderWindow::Closed, this, &GMainWindow::OnStopGame);
    // Update the GUI
    if (ui.action_Single_Window_Mode->isChecked()) {
        game_list->hide();
        game_list_placeholder->hide();
    }

    status_bar_update_timer.start(2000);

    render_window->show();
    render_window->setFocus();

    emulation_running = true;
    if (ui.action_Fullscreen->isChecked()) {
        ShowFullscreen();
    }
    OnStartGame();

    Core::System::GetInstance().GetAppletFactories().erreula.Register(
        "qt", [this](ErrEulaConfig& config) {
            applet_open = true;
            ErrEulaCallback(config);
            std::unique_lock<std::mutex> lock(applet_mutex);
            applet_cv.wait(lock, [&] { return !applet_open; });
        });

    Core::System::GetInstance().GetAppletFactories().swkbd.Register(
        "qt", [this](SoftwareKeyboardConfig& config, std::u16string& text) {
            applet_open = true;
            SwkbdCallback(config, text);
            std::unique_lock<std::mutex> lock(applet_mutex);
            applet_cv.wait(lock, [&] { return !applet_open; });
        });
}

void GMainWindow::ShutdownGame() {
    emu_thread->RequestStop();

    emit EmulationStopping();

    // Wait for emulation thread to complete and delete it
    emu_thread->wait();
    emu_thread = nullptr;

    Camera::QtMultimediaCameraHandler::ReleaseHandlers();

    // The emulation is stopped, so closing the window or not does not matter anymore
    disconnect(render_window, &GRenderWindow::Closed, this, &GMainWindow::OnStopGame);

    // Update the GUI
    ui.action_Start->setEnabled(false);
    ui.action_Start->setText(tr("Start"));
    ui.action_Pause->setEnabled(false);
    ui.action_Stop->setEnabled(false);
    ui.action_Restart->setEnabled(false);
    ui.action_Cheats->setEnabled(false);
    ui.action_Cheat_Search->setEnabled(false);
    ui.action_Set_Play_Coins->setEnabled(false);
    ui.action_Play->setText(tr("Play"));
    ui.action_Play->setEnabled(false);
    ui.action_Record->setText(tr("Record"));
    ui.action_Record->setEnabled(false);
    render_window->hide();
    if (game_list->isEmpty())
        game_list_placeholder->show();
    else
        game_list->show();
    game_list->setFilterFocus();

    Core::Movie::GetInstance().SetPlayFile("");
    Core::Movie::GetInstance().SetRecordFile("");

    // Disable status bar updates
    status_bar_update_timer.stop();
    message_label->setVisible(false);
    emu_speed_label->setVisible(false);
    game_fps_label->setVisible(false);
    emu_frametime_label->setVisible(false);

    emulation_running = false;

    game_title.clear();
    SetupUIStrings();
#ifdef ENABLE_DISCORD_RPC
    Discord_ClearPresence();
    Discord_Shutdown();
#endif
}

void GMainWindow::StoreRecentFile(const QString& filename) {
    UISettings::values.recent_files.prepend(filename);
    UISettings::values.recent_files.removeDuplicates();
    while (UISettings::values.recent_files.size() > max_recent_files_item) {
        UISettings::values.recent_files.removeLast();
    }

    UpdateRecentFiles();
}

void GMainWindow::ErrEulaCallback(ErrEulaConfig& config) {
    if (QThread::currentThread() != thread()) {
        QMetaObject::invokeMethod(this, "ErrEulaCallback", Qt::BlockingQueuedConnection,
                                  Q_ARG(ErrEulaConfig&, config));
        return;
    }

    std::unique_lock<std::mutex> lock(applet_mutex);

    switch (config.error_type) {
    case ErrEulaErrorType::ErrorCode: {
        QMessageBox::critical(this, tr("ErrEula"),
                              tr("Error Code: %1")
                                  .arg(QString::fromStdString(
                                      Common::StringFromFormat("0x%08X", config.error_code))));
        break;
    }
    case ErrEulaErrorType::LocalizedErrorText:
    case ErrEulaErrorType::ErrorText: {
        std::string error = Common::UTF16ToUTF8(config.error_text);
        QMessageBox::critical(
            this, tr("ErrEula"),
            tr("Error Code: %1\n\n%2")
                .arg(QString::fromStdString(Common::StringFromFormat("0x%08X", config.error_code)),
                     QString::fromStdString(error)));
        break;
    }
    case ErrEulaErrorType::Agree:
    case ErrEulaErrorType::Eula:
    case ErrEulaErrorType::EulaDrawOnly:
    case ErrEulaErrorType::EulaFirstBoot: {
        QMessageBox::information(this, tr("ErrEula"), tr("EULA accepted"));
        break;
    }
    }

    config.return_code = ErrEulaResult::Success;
    applet_open = false;
}

void GMainWindow::SwkbdCallback(SoftwareKeyboardConfig& config, std::u16string& text) {
    if (QThread::currentThread() != thread()) {
        QMetaObject::invokeMethod(this, "SwkbdCallback", Qt::BlockingQueuedConnection,
                                  Q_ARG(SoftwareKeyboardConfig&, config),
                                  Q_ARG(std::u16string&, text));
        return;
    }

    std::unique_lock<std::mutex> lock(applet_mutex);
    SoftwareKeyboardDialog dialog(this, config, text);
    dialog.exec();

    applet_open = false;
}

void GMainWindow::UpdateRecentFiles() {
    unsigned int num_recent_files =
        std::min(UISettings::values.recent_files.size(), static_cast<int>(max_recent_files_item));

    for (unsigned int i = 0; i < num_recent_files; i++) {
        QString text = QString("&%1. %2").arg(i + 1).arg(
            QFileInfo(UISettings::values.recent_files[i]).fileName());
        actions_recent_files[i]->setText(text);
        actions_recent_files[i]->setData(UISettings::values.recent_files[i]);
        actions_recent_files[i]->setToolTip(UISettings::values.recent_files[i]);
        actions_recent_files[i]->setVisible(true);
    }

    for (int j = num_recent_files; j < max_recent_files_item; ++j) {
        actions_recent_files[j]->setVisible(false);
    }

    // Grey out the recent files menu if the list is empty
    if (num_recent_files == 0) {
        ui.menu_recent_files->setEnabled(false);
    } else {
        ui.menu_recent_files->setEnabled(true);
    }
}

void GMainWindow::OnGameListLoadFile(QString game_path) {
    BootGame(game_path);
}

void GMainWindow::OnGameListOpenFolder(u64 program_id, GameListOpenTarget target) {
    std::string path;
    std::string open_target;

    switch (target) {
    case GameListOpenTarget::SAVE_DATA: {
        open_target = "Save Data";
        std::string sdmc_dir = Settings::values.sd_card_root.empty()
                                   ? FileUtil::GetUserPath(D_SDMC_IDX)
                                   : Settings::values.sd_card_root + "/";
        path = FileSys::ArchiveSource_SDSaveData::GetSaveDataPathFor(sdmc_dir, program_id);
        break;
    }
    case GameListOpenTarget::APPLICATION:
        open_target = "Application";
        path = Service::AM::GetTitlePath(Service::FS::MediaType::SDMC, program_id) + "content/";
        break;
    case GameListOpenTarget::UPDATE_DATA:
        open_target = "Update Data";
        path = Service::AM::GetTitlePath(Service::FS::MediaType::SDMC, program_id + 0xe00000000) +
               "content/";
        break;
    default:
        LOG_ERROR(Frontend, "Unexpected target {}", static_cast<int>(target));
        return;
    }

    QString qpath = QString::fromStdString(path);

    QDir dir(qpath);
    if (!dir.exists()) {
        QMessageBox::critical(
            this, tr("Error Opening %1 Folder").arg(QString::fromStdString(open_target)),
            tr("Folder does not exist!"));
        return;
    }

    LOG_INFO(Frontend, "Opening {} path for program_id={:016x}", open_target, program_id);

    QDesktopServices::openUrl(QUrl::fromLocalFile(qpath));
}

void GMainWindow::OnGameListOpenDirectory(QString directory) {
    QString path;
    if (directory == "INSTALLED") {
        path = QString::fromStdString(
            (Settings::values.sd_card_root.empty()
                 ? FileUtil::GetUserPath(D_SDMC_IDX)
                 : std::string(Settings::values.sd_card_root + "/")) +
            "Nintendo "
            "3DS/00000000000000000000000000000000/00000000000000000000000000000000/title/00040000");
    } else if (directory == "SYSTEM") {
        path =
            QString::fromStdString(FileUtil::GetUserPath(D_NAND_IDX).c_str() +
                                   std::string("00000000000000000000000000000000/title/00040010"));
    } else {
        path = directory;
    }
    if (!QFileInfo::exists(path)) {
        QMessageBox::critical(this, tr("Error Opening %1").arg(path), tr("Folder does not exist!"));
        return;
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void GMainWindow::OnGameListAddDirectory() {
    QString dir_path = QFileDialog::getExistingDirectory(this, tr("Select Directory"));
    if (dir_path.isEmpty())
        return;
    UISettings::GameDir game_dir{dir_path, false, true};
    if (!UISettings::values.game_dirs.contains(game_dir)) {
        UISettings::values.game_dirs.append(game_dir);
        game_list->PopulateAsync(UISettings::values.game_dirs);
    } else {
        LOG_WARNING(Frontend, "Selected directory is already in the game list");
    }
}

void GMainWindow::OnGameListShowList(bool show) {
    if (emulation_running && ui.action_Single_Window_Mode->isChecked())
        return;
    game_list->setVisible(show);
    game_list_placeholder->setVisible(!show);
}

void GMainWindow::OnMenuLoadFile() {
    QString extensions;
    for (const auto& piece : game_list->supported_file_extensions)
        extensions += "*." + piece + " ";

    QString file_filter = tr("3DS Executable") + " (" + extensions + ")";
    file_filter += ";;" + tr("All Files (*.*)");

    QString filename = QFileDialog::getOpenFileName(this, tr("Load File"),
                                                    UISettings::values.roms_path, file_filter);
    if (!filename.isEmpty()) {
        UISettings::values.roms_path = QFileInfo(filename).path();

        BootGame(filename);
    }
}

void GMainWindow::OnMenuInstallCIA() {
    QStringList filepaths = QFileDialog::getOpenFileNames(
        this, tr("Install CIAs"), UISettings::values.roms_path,
        tr("CTR Importable Archive (*.CIA*)") + ";;" + tr("All Files (*.*)"));
    if (filepaths.isEmpty())
        return;

    ui.action_Install_CIA->setEnabled(false);
    progress_bar->show();

    QtConcurrent::run([&, filepaths] {
        Service::AM::InstallStatus status;
        const auto cia_progress = [&](size_t written, size_t total) {
            emit UpdateProgress(written, total);
        };
        for (const auto current_path : filepaths) {
            status = Service::AM::InstallCIA(current_path.toStdString(), cia_progress);
            emit CIAInstallReport(status, current_path);
        }
        emit CIAInstallFinished();
    });
}

void GMainWindow::OnUpdateProgress(size_t written, size_t total) {
    progress_bar->setValue(
        static_cast<int>(INT_MAX * (static_cast<double>(written) / static_cast<double>(total))));
}

void GMainWindow::OnCIAInstallReport(Service::AM::InstallStatus status, QString filepath) {
    QString filename = QFileInfo(filepath).fileName();
    switch (status) {
    case Service::AM::InstallStatus::Success:
        statusBar()->showMessage(tr("%1 installed").arg(filename));
        break;
    case Service::AM::InstallStatus::ErrorFailedToOpenFile:
        QMessageBox::critical(this, tr("Unable to open File"),
                              tr("Could not open %1").arg(filename));
        break;
    case Service::AM::InstallStatus::ErrorAborted:
        QMessageBox::critical(
            this, tr("Installation aborted"),
            tr("The installation of %1 was aborted. Please see the log for more details")
                .arg(filename));
        break;
    case Service::AM::InstallStatus::ErrorInvalid:
        QMessageBox::critical(this, tr("Invalid File"), tr("%1 is not a valid CIA").arg(filename));
        break;
    case Service::AM::InstallStatus::ErrorEncrypted:
        QMessageBox::critical(this, tr("Encrypted File"),
                              tr("%1 must be decrypted "
                                 "before being used with Citra. A real 3DS is required.")
                                  .arg(filename));
        break;
    }
}

void GMainWindow::OnCIAInstallFinished() {
    progress_bar->hide();
    progress_bar->setValue(0);
    ui.action_Install_CIA->setEnabled(true);
}

void GMainWindow::OnMenuRecentFile() {
    QAction* action = qobject_cast<QAction*>(sender());
    assert(action);

    QString filename = action->data().toString();
    QFileInfo file_info(filename);
    if (file_info.exists()) {
        BootGame(filename);
    } else {
        // Display an error message and remove the file from the list.
        QMessageBox::information(this, tr("File not found"),
                                 tr("File \"%1\" not found").arg(filename));

        UISettings::values.recent_files.removeOne(filename);
        UpdateRecentFiles();
    }
}

void GMainWindow::OnStartGame() {
    Camera::QtMultimediaCameraHandler::ResumeCameras();
    emu_thread->SetRunning(true);
    qRegisterMetaType<Core::System::ResultStatus>("Core::System::ResultStatus");
    qRegisterMetaType<std::string>("std::string");
    connect(emu_thread.get(), &EmuThread::ErrorThrown, this, &GMainWindow::OnCoreError);

    ui.action_Start->setEnabled(false);
    ui.action_Start->setText(tr("Continue"));
    ui.action_Pause->setEnabled(true);
    ui.action_Stop->setEnabled(true);
    ui.action_Restart->setEnabled(true);
    ui.action_Cheats->setEnabled(true);
    ui.action_Cheat_Search->setEnabled(true);
    ui.action_Set_Play_Coins->setEnabled(true);
    ui.action_Play->setDisabled(emulation_running ? Core::Movie::GetInstance().IsRecordingInput()
                                                  : false);
    ui.action_Record->setDisabled(emulation_running ? Core::Movie::GetInstance().IsPlayingInput()
                                                    : false);
}

void GMainWindow::OnPauseGame() {
    emu_thread->SetRunning(false);
    Camera::QtMultimediaCameraHandler::StopCameras();
    ui.action_Start->setEnabled(true);
    ui.action_Pause->setEnabled(false);
    ui.action_Stop->setEnabled(true);
}

void GMainWindow::OnStopGame() {
    ShutdownGame();
}

void GMainWindow::ToggleFullscreen() {
    if (!emulation_running) {
        return;
    }
    if (ui.action_Fullscreen->isChecked()) {
        ShowFullscreen();
    } else {
        HideFullscreen();
    }
}

void GMainWindow::ShowFullscreen() {
    if (ui.action_Single_Window_Mode->isChecked()) {
        UISettings::values.geometry = saveGeometry();
        ui.menubar->hide();
        statusBar()->hide();
        showFullScreen();
    } else {
        UISettings::values.renderwindow_geometry = render_window->saveGeometry();
        render_window->showFullScreen();
    }
}

void GMainWindow::HideFullscreen() {
    if (ui.action_Single_Window_Mode->isChecked()) {
        statusBar()->setVisible(ui.action_Show_Status_Bar->isChecked());
        ui.menubar->show();
        showNormal();
        restoreGeometry(UISettings::values.geometry);
    } else {
        render_window->showNormal();
        render_window->restoreGeometry(UISettings::values.renderwindow_geometry);
    }
}

void GMainWindow::ToggleWindowMode() {
    if (ui.action_Single_Window_Mode->isChecked()) {
        // Render in the main window...
        render_window->BackupGeometry();
        ui.horizontalLayout->addWidget(render_window);
        render_window->setFocusPolicy(Qt::ClickFocus);
        if (emulation_running) {
            render_window->setVisible(true);
            render_window->setFocus();
            game_list->hide();
        }

    } else {
        // Render in a separate window...
        ui.horizontalLayout->removeWidget(render_window);
        render_window->setParent(nullptr);
        render_window->setFocusPolicy(Qt::NoFocus);
        if (emulation_running) {
            render_window->setVisible(true);
            render_window->RestoreGeometry();
            game_list->show();
        }
    }
}

void GMainWindow::ChangeScreenLayout() {
    Settings::LayoutOption new_layout = Settings::LayoutOption::Default;

    if (ui.action_Screen_Layout_Default->isChecked()) {
        new_layout = Settings::LayoutOption::Default;
    } else if (ui.action_Screen_Layout_Single_Screen->isChecked()) {
        new_layout = Settings::LayoutOption::SingleScreen;
    } else if (ui.action_Screen_Layout_Medium_Screen->isChecked()) {
        new_layout = Settings::LayoutOption::MediumScreen;
    } else if (ui.action_Screen_Layout_Large_Screen->isChecked()) {
        new_layout = Settings::LayoutOption::LargeScreen;
    } else if (ui.action_Screen_Layout_Side_by_Side->isChecked()) {
        new_layout = Settings::LayoutOption::SideScreen;
    }

    Settings::values.layout_option = new_layout;
    Settings::Apply();
}

void GMainWindow::ToggleScreenLayout() {
    Settings::LayoutOption new_layout = Settings::LayoutOption::Default;

    switch (Settings::values.layout_option) {
    case Settings::LayoutOption::Default:
        new_layout = Settings::LayoutOption::SingleScreen;
        break;
    case Settings::LayoutOption::SingleScreen:
        new_layout = Settings::LayoutOption::MediumScreen;
        break;
    case Settings::LayoutOption::MediumScreen:
        new_layout = Settings::LayoutOption::LargeScreen;
        break;
    case Settings::LayoutOption::LargeScreen:
        new_layout = Settings::LayoutOption::SideScreen;
        break;
    case Settings::LayoutOption::SideScreen:
        new_layout = Settings::LayoutOption::Default;
        break;
    }

    Settings::values.layout_option = new_layout;
    SyncMenuUISettings();
    Settings::Apply();
}

void GMainWindow::OnSwapScreens() {
    Settings::values.swap_screen = ui.action_Screen_Layout_Swap_Screens->isChecked();
    Settings::Apply();
}

void GMainWindow::OnConfigure() {
    ConfigureDialog configureDialog(this);
    connect(&configureDialog, &ConfigureDialog::languageChanged, this,
            &GMainWindow::OnLanguageChanged);
    auto old_theme = UISettings::values.theme;
    auto result = configureDialog.exec();
    if (result == QDialog::Accepted) {
        configureDialog.applyConfiguration();
        if (UISettings::values.theme != old_theme) {
            UpdateUITheme();
            emit UpdateThemedIcons();
        }
        if (configureDialog.sd_card_root_changed) {
            game_list->PopulateAsync(UISettings::values.game_dirs);
        }
        SyncMenuUISettings();
        config->Save();
    }
}

void GMainWindow::OnCheats() {
    CheatDialog dialog;
    dialog.exec();
}

void GMainWindow::OnCheatSearch() {
    if (cheat_search_window == nullptr)
        cheat_search_window = std::make_shared<CheatSearch>(this);
    cheat_search_window->show();
}

void GMainWindow::OnControlPanel() {
    if (control_panel == nullptr)
        control_panel = std::make_shared<ControlPanel>(this);
    control_panel->show();
}

void GMainWindow::OnSetPlayCoins() {
    bool ok;
    u16 play_coins = static_cast<u16>(
        QInputDialog::getInt(this, tr("Set Play Coins"), tr("Play Coins:"), 0, 0, 300, 1, &ok,
                             Qt::WindowSystemMenuHint | Qt::WindowTitleHint));
    if (ok)
        Service::PTM::GetCurrentModule()->SetPlayCoins(play_coins);
}

void GMainWindow::OnToggleFilterBar() {
    game_list->setFilterVisible(ui.action_Show_Filter_Bar->isChecked());
    if (ui.action_Show_Filter_Bar->isChecked()) {
        game_list->setFilterFocus();
    } else {
        game_list->clearFilter();
    }
}

void GMainWindow::OnPlayMovie() {
    auto& movie = Core::Movie::GetInstance();

    if (movie.IsPlayingInput()) {
        movie.SetPlayFile("");
        ui.action_Play->setText(tr("Play"));
        ui.action_Record->setEnabled(true);
        movie_play_timer.stop();
    } else {
        QString path = QFileDialog::getOpenFileName(this, tr("Play Movie"));
        if (path.isEmpty())
            return;
        if (!std::get<1>(movie.GetFiles()).empty())
            movie.SetRecordFile("");
        movie.SetPlayFile(path.toStdString());
        ui.action_Play->setText(tr("Stop Playing"));
        ui.action_Record->setEnabled(false);
        movie_play_timer.start(500);
    }
}

void GMainWindow::OnRecordMovie() {
    auto& movie = Core::Movie::GetInstance();

    if (movie.IsRecordingInput()) {
        movie.SetPlayFile("");
        ui.action_Play->setEnabled(true);
        ui.action_Record->setText(tr("Record"));
    } else {
        QString path = QFileDialog::getSaveFileName(this, tr("Save Movie"));
        if (path.isEmpty())
            return;
        if (!std::get<0>(movie.GetFiles()).empty())
            movie.SetPlayFile("");
        movie.SetRecordFile(path.toStdString());
        ui.action_Play->setEnabled(false);
        ui.action_Record->setText(tr("Stop Recording"));
    }
}

void GMainWindow::UpdateStatusBar() {
    if (emu_thread == nullptr) {
        status_bar_update_timer.stop();
        return;
    }

    auto results = Core::System::GetInstance().GetAndResetPerfStats();

    if (Settings::values.use_frame_limit) {
        emu_speed_label->setText(tr("Speed: %1% / %2%")
                                     .arg(results.emulation_speed * 100.0, 0, 'f', 0)
                                     .arg(Settings::values.frame_limit));
    } else {
        emu_speed_label->setText(tr("Speed: %1%").arg(results.emulation_speed * 100.0, 0, 'f', 0));
    }
    game_fps_label->setText(tr("Game: %1 FPS").arg(results.game_fps, 0, 'f', 0));
    emu_frametime_label->setText(tr("Frame: %1 ms").arg(results.frametime * 1000.0, 0, 'f', 2));

    emu_speed_label->setVisible(true);
    game_fps_label->setVisible(true);
    emu_frametime_label->setVisible(true);
}

void GMainWindow::OnCoreError(Core::System::ResultStatus result, std::string details) {
    QMessageBox::StandardButton answer;
    QString status_message;
    const QString common_message =
        tr("The game you are trying to load requires additional files from your 3DS to be dumped "
           "before playing.<br/><br/>For more information on dumping these files, please see the "
           "following wiki page: <a "
           "href='https://github.com/valentinvanelslande/citra/wiki/"
           "Dumping-System-Archives-from-a-3DS-Console'>Dumping System "
           "Archives from a 3DS Console</a>.<br/><br/>Would you like to quit "
           "back to the game list? Continuing emulation may result in crashes, corrupted save "
           "data, or other bugs.");
    switch (result) {
    case Core::System::ResultStatus::ErrorSystemFiles: {
        QString message = "Citra was unable to locate a 3DS system archive";
        if (!details.empty()) {
            message.append(tr(": %1. ").arg(details.c_str()));
        } else {
            message.append(". ");
        }
        message.append(common_message);

        answer = QMessageBox::question(this, tr("System Archive Not Found"), message,
                                       QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        status_message = tr("System Archive Missing");
        break;
    }

    case Core::System::ResultStatus::ErrorSharedFont: {
        QString message = tr("Citra was unable to locate the 3DS shared fonts. ");
        message.append(common_message);
        answer = QMessageBox::question(this, tr("Shared Fonts Not Found"), message,
                                       QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        status_message = tr("Shared Font Missing");
        break;
    }

    default:
        answer = QMessageBox::question(
            this, tr("Fatal Error"),
            tr("Citra has encountered a fatal error, please see the log for more details.<br/>"
               "To access the log, Click Open Log Location in the general tab of the configuration "
               "window.<br/><br/>Would you like "
               "to quit back to the game list? "
               "Continuing emulation may result in crashes, corrupted save data, or other bugs."),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        status_message = tr("Fatal Error encountered");
        break;
    }

    if (answer == QMessageBox::Yes) {
        if (emu_thread) {
            ShutdownGame();
        }
    } else {
        // Only show the message if the game is still running.
        if (emu_thread) {
            emu_thread->SetRunning(true);
            message_label->setText(status_message);
            message_label->setVisible(true);
        }
    }
}

void GMainWindow::OnMenuAboutCitra() {
    AboutDialog about{this};
    about.exec();
}

bool GMainWindow::ConfirmClose() {
    if (emu_thread == nullptr || !UISettings::values.confirm_before_closing)
        return true;

    QMessageBox::StandardButton answer =
        QMessageBox::question(this, tr("Citra"), tr("Are you sure you want to close Citra?"),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    return answer != QMessageBox::No;
}

void GMainWindow::closeEvent(QCloseEvent* event) {
    if (!ConfirmClose()) {
        event->ignore();
        return;
    }

    if (!ui.action_Fullscreen->isChecked()) {
        UISettings::values.geometry = saveGeometry();
        UISettings::values.renderwindow_geometry = render_window->saveGeometry();
    }
    UISettings::values.state = saveState();
    UISettings::values.single_window_mode = ui.action_Single_Window_Mode->isChecked();
    UISettings::values.fullscreen = ui.action_Fullscreen->isChecked();
    UISettings::values.display_titlebar = ui.action_Display_Dock_Widget_Headers->isChecked();
    UISettings::values.show_filter_bar = ui.action_Show_Filter_Bar->isChecked();
    UISettings::values.show_status_bar = ui.action_Show_Status_Bar->isChecked();
    UISettings::values.first_start = false;

    game_list->SaveInterfaceLayout();
    SaveHotkeys();

    // Shutdown session if the emu thread is active...
    if (emu_thread != nullptr)
        ShutdownGame();

    render_window->close();
    multiplayer_state->Close();
    QWidget::closeEvent(event);
}

static bool IsSingleFileDropEvent(QDropEvent* event) {
    const QMimeData* mimeData = event->mimeData();
    return mimeData->hasUrls() && mimeData->urls().length() == 1;
}

void GMainWindow::dropEvent(QDropEvent* event) {
    if (IsSingleFileDropEvent(event) && ConfirmChangeGame()) {
        const QMimeData* mimeData = event->mimeData();
        QString filename = mimeData->urls().at(0).toLocalFile();
        BootGame(filename);
    }
}

void GMainWindow::dragEnterEvent(QDragEnterEvent* event) {
    if (IsSingleFileDropEvent(event)) {
        event->acceptProposedAction();
    }
}

void GMainWindow::dragMoveEvent(QDragMoveEvent* event) {
    event->acceptProposedAction();
}

bool GMainWindow::ConfirmChangeGame() {
    if (emu_thread == nullptr)
        return true;

    auto answer = QMessageBox::question(
        this, tr("Citra"),
        tr("Are you sure you want to stop the emulation? Any unsaved progress will be lost."),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    return answer != QMessageBox::No;
}

void GMainWindow::filterBarSetChecked(bool state) {
    ui.action_Show_Filter_Bar->setChecked(state);
    emit(OnToggleFilterBar());
}

void GMainWindow::UpdateUITheme() {
    QStringList theme_paths(default_theme_paths);
    if (UISettings::values.theme != UISettings::themes[0].second &&
        !UISettings::values.theme.isEmpty()) {
        QString theme_uri(":" + UISettings::values.theme + "/style.qss");
        QFile f(theme_uri);
        if (!f.exists()) {
            LOG_ERROR(Frontend, "Unable to set style, stylesheet file not found");
        } else {
            f.open(QFile::ReadOnly | QFile::Text);
            QTextStream ts(&f);
            qApp->setStyleSheet(ts.readAll());
            GMainWindow::setStyleSheet(ts.readAll());
        }
        theme_paths.append(QStringList{":/icons/default", ":/icons/" + UISettings::values.theme});
        QIcon::setThemeName(":/icons/" + UISettings::values.theme);
    } else {
        qApp->setStyleSheet("");
        GMainWindow::setStyleSheet("");
        theme_paths.append(QStringList{":/icons/default"});
        QIcon::setThemeName(":/icons/default");
    }
    QIcon::setThemeSearchPaths(theme_paths);
}

void GMainWindow::LoadTranslation() {
    // If the selected language is English, no need to install any translation
    if (UISettings::values.language == "en") {
        return;
    }

    bool loaded;

    if (UISettings::values.language.isEmpty()) {
        // If the selected language is empty, use system locale
        loaded = translator.load(QLocale(), "", "", ":/languages/");
    } else {
        // Otherwise load from the specified file
        loaded = translator.load(UISettings::values.language, ":/languages/");
    }

    if (loaded) {
        qApp->installTranslator(&translator);
    } else {
        UISettings::values.language = "en";
    }
}

void GMainWindow::OnLanguageChanged(const QString& locale) {
    if (UISettings::values.language != "en") {
        qApp->removeTranslator(&translator);
    }

    UISettings::values.language = locale;
    LoadTranslation();
    ui.retranslateUi(this);
    SetupUIStrings();
}

void GMainWindow::SetupUIStrings() {
    if (game_title.isEmpty()) {
        setWindowTitle(tr("Citra %1").arg(Common::g_build_fullname));
    } else {
        setWindowTitle(tr("Citra %1| %2").arg(Common::g_build_fullname, game_title));
    }
}

void GMainWindow::SyncMenuUISettings() {
    ui.action_Screen_Layout_Default->setChecked(Settings::values.layout_option ==
                                                Settings::LayoutOption::Default);
    ui.action_Screen_Layout_Single_Screen->setChecked(Settings::values.layout_option ==
                                                      Settings::LayoutOption::SingleScreen);
    ui.action_Screen_Layout_Medium_Screen->setChecked(Settings::values.layout_option ==
                                                      Settings::LayoutOption::MediumScreen);
    ui.action_Screen_Layout_Large_Screen->setChecked(Settings::values.layout_option ==
                                                     Settings::LayoutOption::LargeScreen);
    ui.action_Screen_Layout_Side_by_Side->setChecked(Settings::values.layout_option ==
                                                     Settings::LayoutOption::SideScreen);
    ui.action_Screen_Layout_Swap_Screens->setChecked(Settings::values.swap_screen);
}

#ifdef main
#undef main
#endif

int main(int argc, char* argv[]) {
    // Init settings params
    QCoreApplication::setOrganizationName("Citra team");
    QCoreApplication::setApplicationName("Citra");

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QApplication app(argc, argv);

    // Qt changes the locale and causes issues in float conversion using std::to_string() when
    // generating shaders
    setlocale(LC_ALL, "C");

    GMainWindow main_window;
    // After settings have been loaded by GMainWindow, apply the filter
    Log::Filter log_filter;
    log_filter.ParseFilterString(Settings::values.log_filter);
    Log::SetGlobalFilter(log_filter);
    FileUtil::CreateFullPath(FileUtil::GetUserPath(D_LOGS_IDX));
    Log::AddBackend(
        std::make_unique<Log::FileBackend>(FileUtil::GetUserPath(D_LOGS_IDX) + LOG_FILE));

    // Register CameraFactory
    Camera::RegisterFactory("image", std::make_unique<Camera::StillImageCameraFactory>());
    Camera::RegisterFactory("qt", std::make_unique<Camera::QtMultimediaCameraFactory>());
    Camera::QtMultimediaCameraHandler::Init();
    LOG_INFO(Frontend, "Citra version: {}", Common::g_build_version);
    main_window.show();
    return app.exec();
}
