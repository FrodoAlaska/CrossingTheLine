pushd build 

cmake --build . --config Debug --parallel 8
if %ERRORLEVEL% NEQ 0 goto error

.\Debug\game.exe 

popd

:error 
echo "Error occured while trying to execute the game"
popd
