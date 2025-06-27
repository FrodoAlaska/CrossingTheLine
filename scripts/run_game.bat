pushd build 

cmake --build . --config Release --parallel 8
if %ERRORLEVEL% NEQ 0 goto error

.\Release\game.exe 

popd

:error 
echo "Error occured while trying to execute the game"
popd
