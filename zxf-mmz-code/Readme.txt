

1. Makefile 需要成自己tool chain 的路径

2. 目录结构
   mmz-code/ 下面的Makefile 编译出mmz的驱动
   mmz-code/app/static/ 下面的Makefile编译出mmz .a 静态库文件
   mmz-code/app/lib_tets 下面则是测试 mmz .a 库文件的测试sample code
   mmz-code/app 下面的app_test 是我开发mmz驱动的测试代码

   mmz-code/app/include 下面是需要包含的头文件


   git clone ssh://zhuxf@192.168.9.239:29418/polaris_ck810_860 && scp -p -P 29418 zhuxf@192.168.9.239:hooks/commit-msg polaris_ck810_860/.git/hooks/
