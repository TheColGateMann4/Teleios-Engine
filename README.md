My Graphic Engine made with DirectX 12


Libraries:
-assimp
-DirectXTex
-DirectXTK12
-imgui

How i added imgui:
1. Made other project where i added every imgui file that I needed, and made that project into library.
2. Added that project to this project and added it as reference by:
   - clicking on this project>Add..>Add refererence> Selected library project.

Why i done it this way?
I like having all warnings turned on and imgui just spammed one thousand of them, i don't like it

How it could be done differently?
1. Add imgui library files to this project and that's it. Besides annoying warnings everything is the same
