$progressPreference = 'silentlyContinue'
Invoke-WebRequest -Uri "https://raw.githubusercontent.com/acnleditor2/citra/master/updater.ps1" -OutFile "updater.ps1"
if (-not (Test-Path updater.cfg)) {
    Add-Content updater.cfg 0
    Add-Content updater.cfg 0
}
try {
cls
Write-Host "Starting..."
$type = (Get-Content updater.cfg)[1]
if ($type -cgt 1) {
    throw "Invalid build type option"
}
$local_commit = (Get-Content updater.cfg)[0]
if ($local_commit -eq $latest_commit) {
if (Test-Path citra-qt.exe) {
    start citra-qt.exe
}
    Break
}
$releases = "https://api.github.com/repos/acnleditor2/citra/releases"
$latest_release = (Invoke-WebRequest $releases | ConvertFrom-Json)[0]
$latest_commit = $latest_release.target_commitish.Remove(7, 33)
$tag = $latest_release.tag_name
$assets = $latest_release.assets_url
if ($type -eq 0) {
    $file = (Invoke-WebRequest $assets | ConvertFrom-Json)[0].name
} elseif ($type -eq 1) {
    $file = (Invoke-WebRequest $assets | ConvertFrom-Json)[1].name
}
$download = "https://github.com/acnleditor2/citra/releases/download/$tag/$file"
cls
Write-Host "Downloading Citra..."
Invoke-WebRequest $download -Out "citra.zip"
cls
Write-Host "Extracting Citra..."
Expand-Archive "citra.zip" -Force
cls
if (Test-Path platforms) {
    Remove-Item "platforms" -Force -Recurse
}
if ($type -eq 0) {
    Move-Item "citra/head-mingw/*" "./" -Force
    Remove-Item "citra/head-mingw" -Force -Recurse
} else {
    Move-Item "citra/head/*" "./" -Force
    Remove-Item "citra/head" -Force -Recurse
}
Remove-Item "citra" -Force -Recurse
Remove-Item "citra.zip" -Force
start citra-qt.exe
} catch {
  Write-Host $error[0]
  Pause
}
