echo "Setting up folder for build files..."
mkdir -p out
cd out
if  [ "`uname -s`" = "Darwin" ]; then
  TRT_GENERATOR="-GXcode"
else
  TRT_GENERATOR=
fi

cmake .. ${TRT_GENERATOR} $@
echo Finished successfully!
