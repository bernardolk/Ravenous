cd "c:\repositories\ravenous\src"
dir -recurse -include *.h, *.cpp | Get-Content | Measure-Object -Line