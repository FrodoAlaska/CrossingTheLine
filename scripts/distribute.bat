pushd build 

cmake .. -DDISTRIBUTION_BUILD=1
cmake --build . --config Release --parallel 12 
xcopy Release\cross.exe .\

"C:\Program Files\7-Zip\7z.exe" a -tzip "CrossingTheLine-Win32.zip" dialogue.txt res\ levels\ cross.exe 

popd
