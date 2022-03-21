copy "..\Builds\VisualStudio2019\x64\Release\Dynamic Library\dawdreamer.dll" "%pythonLocation%\dawdreamer.pyd"
copy "..\thirdparty\libfaust\win-x64\Release\bin\faust.dll" "%pythonLocation%\faust.dll"

python -m pytest -v .