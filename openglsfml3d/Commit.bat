@echo off

set /p input=Commit name: 

git add *.cpp
git add *.h
git add *.md
git add *.vert
git add *.frag
git add LICENSE
git add maps
git add textures
git add objects
git commit -m"%input%"
git push -u