@echo off

echo Generating build files...
mkdir out
cd out
cmake -S ..
cd ..
echo Finished successfully!
pause
