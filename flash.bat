@echo off
setlocal

REM ================== SETTINGS ==================
set CHIP=esp32c3
set BAUD=921600

set BOOTLOADER=bootloader.bin
set PARTITIONS=partitions.bin
set APP=firmware_hm10_esp32c3.bin

set ESPTOOL=esptool.exe
set LOG=flash_log.txt
REM ==============================================

echo ===============================================
echo ESP32-C3 FACTORY FLASH
echo ===============================================
echo.

REM ---- Check required files ----
for %%F in (%BOOTLOADER% %PARTITIONS% %APP% %ESPTOOL%) do (
  if not exist "%%F" (
    echo ERROR: File %%F not found!
    pause
    exit /b 1
  )
)

echo.
echo Available COM ports:
mode | findstr "COM"
echo.

set /p PORT=Enter ESP32 COM port (example COM6): 

if "%PORT%"=="" (
  echo ERROR: No COM port entered
  pause
  exit /b 1
)

echo.
echo Using port: %PORT%
echo.

REM ---- Log header ----
echo Flash started at %DATE% %TIME% > %LOG%
echo Port: %PORT% >> %LOG%
echo Baud: %BAUD% >> %LOG%
echo. >> %LOG%

REM ---- Erase flash ----
"%ESPTOOL%" --chip %CHIP% ^
  --port %PORT% ^
  --baud %BAUD% ^
  erase_flash >> %LOG% 2>&1

REM ---- Write flash ----
"%ESPTOOL%" --chip %CHIP% ^
  --port %PORT% ^
  --baud %BAUD% ^
  write_flash ^
  0x0 %BOOTLOADER% ^
  0x8000 %PARTITIONS% ^
  0x10000 %APP% >> %LOG% 2>&1

if %ERRORLEVEL% NEQ 0 (
  echo.
  echo FLASH FAILED!
  echo See log: %LOG%
  pause
  exit /b 1
)

echo.
echo ===============================================
echo FLASH COMPLETED SUCCESSFULLY
echo ===============================================
echo.

pause
