$progressPreference = 'silentlyContinue'
[System.Net.ServicePointManager]::SecurityProtocol = [System.Net.SecurityProtocolType]::Tls12
Write-Host "Updating to latest unstable version..."
Invoke-WebRequest -Uri "https://raw.githubusercontent.com/valentinvanelslande/citra/master/updater.ps1" -OutFile "updater.ps1"
cls
if (!(Test-Path updater.cfg)) {
    Add-Content -Path updater.cfg -Value "mingw"
    Add-Content -Path updater.cfg -Value "0"
}
try {
cls
Write-Host "Starting..."
$type = (Get-Content "updater.cfg").Split("`n")[0]
$valid = $false
if ($type -eq "mingw") {
    $valid = $true
}
if ($type -eq "msvc") {
    $valid = $true
}
if (!$valid) {
    throw "Invalid build type option"
}
$latest_release = (Invoke-WebRequest "https://api.github.com/repos/valentinvanelslande/citra/releases" | ConvertFrom-Json)[0]
$local_commit = (Get-Content updater.cfg)[0]
$latest_commit = $latest_release.target_commitish.Remove(7, 33)
if ($local_commit -eq $latest_commit) {
if (Test-Path citra-qt.exe) {
    start citra-qt.exe
}
    break
}
$tag = $latest_release.tag_name
$assets = (Invoke-WebRequest $latest_release.assets_url | ConvertFrom-Json)
$found = $false
for ($i = 0; $i -lt $assets.Count; ++$i) {
    if ($assets[$i].name.StartsWith("citra-windows-$type") -and $assets[$i].name.EndsWith(".zip")) {
        $found = $true
        $file = $assets[$i].name
        $size = $assets[$i].size / 1Mb
        break
    }
}
if (!$found) {
    switch ($type) {
        "mingw" { $build_string = "MinGW build" }
        "msvc" { $build_string = "MSVC build" }
    }
    Write-Host "No $build_string for release $tag!"
    Pause
    break
}
$download = "https://github.com/valentinvanelslande/citra/releases/download/$tag/$file"
cls
Write-Host "Downloading Citra (Size: $size MB)..."
Invoke-WebRequest $download -Out "citra.zip"
cls
Write-Host "Extracting Citra..."
Expand-Archive "citra.zip" -Force
cls
if (Test-Path "platforms") {
    Remove-Item "platforms" -Force -Recurse
}
if (Test-Path "mediaservice") {
    Remove-Item "mediaservice" -Force -Recurse
}
if ($type -eq "mingw") {
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
  if ($error[0].ToString().Contains("API rate limit exceeded")) {
      Write-Host "Bandwidth limit (60MB) exceeded, wait." -ForegroundColor Red
  } else {
      Write-Host $error[0] -ForegroundColor Red
  }
  Pause
}
