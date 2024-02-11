@echo off

echo Downloading library dependencies for samples...
git submodule init || (exit /b)
git submodule update || (exit /b)
echo Setting up folder for build files...
if not exist ".\out" mkdir out || (exit /b)
cd out || (exit /b)
echo Generating build files...
cmake .. ||  (exit /b)
cd ..
echo Finished successfully!
pause
