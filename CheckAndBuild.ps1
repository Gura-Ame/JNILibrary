$headerDir = (Get-Location).Path
$javaFilePath = "C:\Users\NOBTG\Documents\Projects\MemoryStudy\src\main\java\org\example\Native.java"
$compiler="C:\msys64\ucrt64\bin\g++.exe"

# 取得 java 目錄路徑，從 javaFilePath 往上找直到遇到 "java"
function Get-JavaFolder([string]$filePath) {
    $dir = Split-Path $filePath -Parent
    while ($dir -and (Split-Path $dir -Leaf) -ne "java") {
        $dir = Split-Path $dir -Parent
    }
    if ($dir -and (Split-Path $dir -Leaf) -eq "java") {
        return $dir
    } else {
        throw "Cannot find 'java' folder in path hierarchy of $filePath"
    }
}

$srcDir = Get-JavaFolder $javaFilePath

# 從 javaFilePath 取類名（檔名不帶副檔名）
$javaFileName = [System.IO.Path]::GetFileNameWithoutExtension($javaFilePath)

# header 檔名，照 javac 預設格式 (包名用底線，類名)
# 例如 org.example.Main -> org_example_Main.h
# 這邊先用簡單版本（用包名加類名）
# 先取得 java 檔案相對 src 目錄的路徑
$relativePath = Resolve-Path $javaFilePath | ForEach-Object {
    $_.Path.Substring($srcDir.Length + 1)
}

# 把路徑分割成資料夾，取出 package 路徑與類名
$relativeDirs = Split-Path $relativePath -Parent
$packageParts = $relativeDirs -split '[\\/]'

# header name = [package parts joined by _] + _ + java file name + .h
# 如果沒有 package 就直接類名.h
if ($packageParts -and $packageParts[0] -ne '') {
    $headerBaseName = ($packageParts -join "_") + "_" + $javaFileName
} else {
    $headerBaseName = $javaFileName
}

# header 的路徑與暫存、備份檔案路徑
$headerPath = Join-Path $headerDir ($headerBaseName + ".h")
$beforeHeaderPath = Join-Path $headerDir ($headerBaseName + "_tmp.h")
$oldBackupPath = Join-Path $headerDir ($headerBaseName + "_old.h")

$javac = Join-Path $env:JAVA_HOME "bin\javac.exe"

# 先刪除暫存檔（如果存在）
if (Test-Path $beforeHeaderPath) { Remove-Item $beforeHeaderPath -Force }
if (Test-Path $oldBackupPath) { Remove-Item $oldBackupPath -Force }

Copy-Item $headerPath $beforeHeaderPath -Force

# 1. 執行 javac -h 產生 header，路徑給資料夾
& $javac -h $headerDir $javaFilePath

if (-not (Test-Path $beforeHeaderPath)) {
    Write-Error "Failed to generate new header file: $beforeHeaderPath"
    exit 1
}

# 讀取內容比對
$oldContent = Get-Content $beforeHeaderPath -Raw
$newContent = Get-Content $headerPath -Raw

if ($oldContent -eq $newContent) {
    Write-Output "C header check success."
    Remove-Item $beforeHeaderPath -Force

    cmake -B build_ninja -G Ninja "-DCMAKE_CXX_COMPILER=$compiler"
    cmake --build build_ninja

    $dllFile = Join-Path $headerDir ($headerBaseName + ".dll")
    $destDll = Join-Path (Split-Path $javaFilePath -Parent) ($headerBaseName + ".dll")

    if (Test-Path $dllFile) {
        Copy-Item -Path $dllFile -Destination $destDll -Force
        Write-Output "Copied DLL from $dllFile to $destDll"

        Remove-Item $dllFile -Force
        Write-Output "Deleted $dllFile"
    } else {
        Write-Warning "Source DLL $dllFile not found."
    }
} else {
    Write-Error "C header check fail."

    $answer = Read-Host "New header differs. Replace the old header? (y/n)"
    if ($answer.ToLower() -eq 'y') {
        # 備份舊檔
        if (Test-Path $beforeHeaderPath) {
            Move-Item -Path $beforeHeaderPath -Destination $oldBackupPath -Force
        }

        Write-Output "Old header backed up as $oldBackupPath"
        Write-Output "New header replaced the original."
    } else {
        # 不替換，刪除暫存檔
        Remove-Item $beforeHeaderPath -Force
        Write-Output "Old header preserved, tmp file removed."
    }
}

# 要刪除的 class 檔案路徑
$classFilePath = [System.IO.Path]::ChangeExtension($javaFilePath, ".class")

if (Test-Path $classFilePath) {
    Remove-Item $classFilePath -Force
    Write-Output "Deleted $classFilePath"
} else {
    Write-Output "Class file $classFilePath not found, nothing to delete."
}