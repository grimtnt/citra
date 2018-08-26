// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <QLabel>
#include <QMainWindow>
#include <QTimer>
#include <QTranslator>
#include "citra_qt/swkbd.h"
#include "core/core.h"
#include "core/hle/applets/erreula.h"
#include "core/hle/applets/swkbd.h"
#include "core/hle/service/am/am.h"
#include "ui_main.h"

class AboutDialog;
class CheatSearch;
class Config;
class ControlPanel;
class ClickableLabel;
class EmuThread;
class GameList;
enum class GameListOpenTarget;
class GameListPlaceholder;
class GImageInfo;
class GRenderWindow;
class MultiplayerState;
template <typename>
class QFutureWatcher;
class QProgressBar;

class GMainWindow : public QMainWindow {
    Q_OBJECT

    /// Max number of recently loaded items to keep track of
    static const int max_recent_files_item = 10;

public:
    void filterBarSetChecked(bool state);
    void UpdateUITheme();

    GMainWindow();
    ~GMainWindow();

    GameList* game_list;

signals:

    /**
     * Signal that is emitted when a new EmuThread has been created and an emulation session is
     * about to start. At this time, the core system emulation has been initialized, and all
     * emulation handles and memory should be valid.
     *
     * @param emu_thread Pointer to the newly created EmuThread (to be used by widgets that need to
     *      access/change emulation state).
     */
    void EmulationStarting(EmuThread* emu_thread);

    /**
     * Signal that is emitted when emulation is about to stop. At this time, the EmuThread and core
     * system emulation handles and memory are still valid, but are about become invalid.
     */
    void EmulationStopping();

    void UpdateProgress(size_t written, size_t total);
    void CIAInstallReport(Service::AM::InstallStatus status, QString filepath);
    void CIAInstallFinished();
    // Signal that tells widgets to update icons to use the current theme
    void UpdateThemedIcons();

private:
    void InitializeWidgets();
    void InitializeRecentFileMenuActions();
    void InitializeHotkeys();

    void SetDefaultUIGeometry();
    Q_INVOKABLE void ErrEulaCallback(HLE::Applets::ErrEulaConfig& config);
    Q_INVOKABLE void SwkbdCallback(HLE::Applets::SoftwareKeyboardConfig& config,
                                   std::u16string& text);
    void SyncMenuUISettings();
    void RestoreUIState();

    void ConnectWidgetEvents();
    void ConnectMenuEvents();

    bool LoadROM(const QString& filename);
    void BootGame(const QString& filename);
    void ShutdownGame();

    /**
     * Stores the filename in the recently loaded files list.
     * The new filename is stored at the beginning of the recently loaded files list.
     * After inserting the new entry, duplicates are removed meaning that if
     * this was inserted from \a OnMenuRecentFile(), the entry will be put on top
     * and remove from its previous position.
     *
     * Finally, this function calls \a UpdateRecentFiles() to update the UI.
     *
     * @param filename the filename to store
     */
    void StoreRecentFile(const QString& filename);

    /**
     * Updates the recent files menu.
     * Menu entries are rebuilt from the configuration file.
     * If there is no entry in the menu, the menu is greyed out.
     */
    void UpdateRecentFiles();

    /**
     * If the emulation is running,
     * asks the user if he really want to close the emulator
     *
     * @return true if the user confirmed
     */
    bool ConfirmClose();
    bool ConfirmChangeGame();
    void closeEvent(QCloseEvent* event) override;

private slots:
    void OnStartGame();
    void OnPauseGame();
    void OnStopGame();
    /// Called whenever a user selects a game in the game list widget.
    void OnGameListLoadFile(QString game_path);
    void OnGameListOpenFolder(u64 program_id, GameListOpenTarget target);
    void OnGameListOpenDirectory(QString path);
    void OnGameListAddDirectory();
    void OnGameListShowList(bool show);
    void OnMenuLoadFile();
    void OnMenuInstallCIA();
    void OnUpdateProgress(size_t written, size_t total);
    void OnCIAInstallReport(Service::AM::InstallStatus status, QString filepath);
    void OnCIAInstallFinished();
    void OnMenuRecentFile();
    void OnConfigure();
    void OnSetPlayCoins();
    void OnCheats();
    void OnCheatSearch();
    void OnControlPanel();
    void OnToggleFilterBar();
    void OnDisplayTitleBars(bool);
    void ToggleFullscreen();
    void ChangeScreenLayout();
    void ToggleScreenLayout();
    void OnSwapScreens();
    void ShowFullscreen();
    void HideFullscreen();
    void ToggleWindowMode();
    void OnRecordMovie();
    void OnPlayMovie();
    void OnStopRecordingPlayback();
    void OnCoreError(Core::System::ResultStatus, std::string);
    /// Called whenever a user selects Help->About Citra
    void OnMenuAboutCitra();
    void OnLanguageChanged(const QString& locale);

private:
    bool ValidateMovie(const QString& path, u64 program_id = 0);
    Q_INVOKABLE void OnMoviePlaybackCompleted();
    void UpdateStatusBar();
    void LoadTranslation();
    void SetupUIStrings();
    void RetranslateStatusBar();

    Ui::MainWindow ui;

    GRenderWindow* render_window;

    GameListPlaceholder* game_list_placeholder;

    // Status bar elements
    QProgressBar* progress_bar = nullptr;
    QLabel* message_label = nullptr;
    QLabel* emu_speed_label = nullptr;
    QLabel* game_fps_label = nullptr;
    QLabel* emu_frametime_label = nullptr;
    QTimer status_bar_update_timer;

    QTimer movie_play_timer;

    MultiplayerState* multiplayer_state = nullptr;
    std::unique_ptr<Config> config;

    // Whether emulation is currently running in Citra.
    bool emulation_running = false;
    std::unique_ptr<EmuThread> emu_thread;
    // The title of the game currently running
    QString game_title;
    // The path to the game currently running
    QString game_path;

    // Movie
    bool movie_record_on_start = false;
    QString movie_record_path;

    // Variables used to sleep the application when a Qt applet is open.
    bool applet_open = false;
    std::mutex applet_mutex;
    std::condition_variable applet_cv;

    // HLE Qt Applets
    std::unique_ptr<SoftwareKeyboardDialog> swkbd;

    std::shared_ptr<ControlPanel> control_panel;
    std::shared_ptr<CheatSearch> cheat_search_window;

    QAction* actions_recent_files[max_recent_files_item];

    QTranslator translator;

    // stores default icon theme search paths for the platform
    QStringList default_theme_paths;

protected:
    void dropEvent(QDropEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
};

Q_DECLARE_METATYPE(size_t);
Q_DECLARE_METATYPE(Service::AM::InstallStatus);
