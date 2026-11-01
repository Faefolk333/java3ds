$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$toolsDir = Join-Path $root 'work/cia-tools'
New-Item -ItemType Directory -Force $toolsDir | Out-Null
$packages = @(
  @('makerom184','https://github.com/3DSGuy/Project_CTR/releases/download/makerom-v0.18.4/makerom-v0.18.4-win_x86_64.zip','d743507dc66d4c6f8efcce05ff8f5f47163a60e54e0e0c57fa12ec2f54acdf2f'),
  @('bannertool','https://github.com/carstene1ns/3ds-bannertool/releases/download/1.2.3/bannertool-1.2.3-windows.zip','d0f277bd875e2ad411844378155d27afe124780fcccb d81d9a594e886e2dd3c4'.Replace(' ','')),
  @('ctrtool','https://github.com/3DSGuy/Project_CTR/releases/download/ctrtool-v1.3.0/ctrtool-v1.3.0-win_x64.zip','8031dff3be72d0adb250fae1f969f27627e12a89ebc6dd074a15a75f87ddc949')
)
foreach ($item in $packages) {
  $archive = Join-Path $toolsDir ($item[0]+'.zip')
  if (!(Test-Path -LiteralPath $archive)) { Invoke-WebRequest $item[1] -OutFile $archive }
  if ((Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash -ne $item[2]) { throw "Checksum mismatch: $($item[0])" }
  Expand-Archive -LiteralPath $archive -DestinationPath (Join-Path $toolsDir $item[0]) -Force
}
Write-Host 'CIA tools ready.'
