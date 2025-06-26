pushd build 

cmake .. -DDISTRIBUTION_BUILD=1
cmake --build . --config Release --parallel 12 
xcopy Release\game.exe /f .\

"C:\Program Files\7-Zip\7z.exe" a -tzip "CrossingTheLine.zip" dialogue.txt res\ levels\ game.exe 

popd
