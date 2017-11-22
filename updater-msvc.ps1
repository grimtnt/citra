Write-Host WIP
$releases = "https://api.github.com/repos/acnleditor2/citra/releases"
$tag = (Invoke-WebRequest $releases | ConvertFrom-Json)[0].tag_name
$assets = (Invoke-WebRequest $releases | ConvertFrom-Json)[0].assets_url
$file = (Invoke-WebRequest $assets | ConvertFrom-Json)[1].name
$download = "https://github.com/acnleditor2/citra/releases/download/$tag/$file"
$name = $file.Split(".")[0]
$dir = "$name"
Invoke-WebRequest $download -Out "citra.zip"
Expand-Archive "citra.zip"
Move-Item "citra/head/*" ".\"
Remove-Item "citra/head" -Recurse
Remove-Item "citra"
Remove-Item "citra.zip"
start "citra-qt.exe"
