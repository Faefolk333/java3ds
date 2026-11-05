$ErrorActionPreference='Stop'
$root=$PSScriptRoot
$jdk='C:/Users/Administrator/Documents/Codex/2026-09-26/referenced-chatgpt-conversation-this-is-an/work/jdk/jdk-17.0.20.1+1/bin'
if($env:JAVA_HOME){$jdk=Join-Path $env:JAVA_HOME bin}
New-Item -ItemType Directory -Force "$root/work/classes","$root/work/demo","$root/romfs/demo","$root/romfs/java" | Out-Null
if(!(Test-Path "$root/work/minijvm_rt.jar")){Copy-Item "$root/romfs/java/minijvm_rt.jar" "$root/work/minijvm_rt.jar"}
$classOutput=Join-Path $root ('work/classes-'+[Guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Force $classOutput | Out-Null
$sources=Get-ChildItem "$root/java" -Recurse -Filter '*.java' | ForEach-Object FullName
& "$jdk/javac.exe" --release 8 -encoding UTF-8 -cp "$root/work/minijvm_rt.jar" -d $classOutput $sources
if($LASTEXITCODE){throw 'MIDP compilation failed'}
& "$jdk/jar.exe" --create --file "$root/work/midp.jar" -C $classOutput .
& "$jdk/javac.exe" --release 8 -encoding UTF-8 -cp "$root/work/midp.jar" -d "$root/work/demo" "$root/tests/LauncherCheck.java"
if($LASTEXITCODE){throw 'Demo compilation failed'}
Copy-Item "$root/tests/check.txt" "$root/work/demo/check.txt"
& "$jdk/jar.exe" --create --file "$root/romfs/demo/LauncherCheck.jar" --manifest "$root/tests/demo.mf" -C "$root/work/demo" .
if($LASTEXITCODE){throw 'Demo packaging failed'}
Copy-Item "$root/work/midp.jar","$root/work/minijvm_rt.jar" "$root/romfs/java"
