建立自己的分支相关操作   
1、建立文件夹存放项目，并进入文件夹  
mkdir xiaozhong  
cd xiaozhong  
2、克隆master到自己的文件夹  
git clone  http://10.0.0.253:8081/smart_navigation/navigation_zhao.git  //从管理员master那里克隆项目  
3、进入到clone的项目中  
cd navigation_zhao  
4、建立自己的分支，eg：创建xiaozhong的分支  并切换到自己的分支  
git checkout -b xiaozhong    //创建自己的分支，并切换到此分支上，命名为xiaozhong    
4、随便修改文件   
5、
git add . //加入跟踪  
6、加入到仓库
git commit -m “注释”   
7、推到gitlab的分支上，网上就可以看到了  
git push origin xiaozhong //推送到远程服务器xiaozhong分支上  
8、如果代码改好了，在gitlab上提出合并请求，就可以合并到master