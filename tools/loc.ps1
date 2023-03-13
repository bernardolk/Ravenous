cd "c:\dev\ravenous\src"
dir -recurse -include *.h, *.cpp | Get-Content | Measure-Object -Line