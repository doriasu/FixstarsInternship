# Fixstars summer internship2019
2019年夏季に行われたFixstars internshipにて制作したbeaglebone上で動作する組み込みOSであるQNXむけのカメラドライバです。
Beaglebone blackにarducamを繋いで使います。
## ブランチ  
### Camera  
カメラを用いて撮影可能なコードです。各ファイルをmakeし、./satsuei <ファイル名> <解像度>　で写真の撮影、保存が可能です。  
解像度は、  
* 160*120
* 176*144  
* 320*240  
* 640*480  
* 800*600  
* 1024*768  
* 1280*1024  
* 1600*1200  
とうつと認識します。  
  
### Lchika  
BeagleBone black備え付けのLEDをつけるコードです。  
./led 1010とうつと左から1番目、3番目のLEDが点灯します。  

### MessagePassing  
QNXの機能であるMessagePassingを用いた足し算、引き算プログラムです。  

### ResourceManager  
QNXの機能であるResourceManagerを利用し、カメラを操作するための、/dev/Cameraを生成し、それを用いた解像度変更や写真撮影をできるようにするプログラムです。  
makeした上で./Cameraを実行すると /dev/Cameraが見えるようになるので /dev/Camera/satsuei にコマンドを送ると撮影でき、
 /dev/Camera/kaizoudo にコマンドを送るとカメラの解像度の変更が可能です。  
 
 ### copy  
 sourceをコピーしdstに書き込むプログラムです。  
 
 ### slide  
 成果発表会で用いたスライドです。  
 
