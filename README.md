因为实战中Mimikatz的工具被各大安全厂商视为眼中钉，所以自己实现了一个对于LSASS进程的hash转存，代码里有用到了通过DefineDosDevice来绕过目标系统开启了PPL而无法dump hash的情况


###### Usage:

将编译后的文件放到目标靶机上以管理员身份运行，即可获得dump文件
![image-20230325104311163](https://user-images.githubusercontent.com/83112602/227687343-6ae57dd3-87ec-435c-a4a5-5a36c13aeea1.png)
成功dump出hash![image-20230325104437248](https://user-images.githubusercontent.com/83112602/227687375-87adae5e-8e44-4e3a-82d6-9176363f3329.png)



###### 杀软评测

静态的话目前都没什么问题 实际测试中卡巴会检测到后续动态行为

火绒

![image-20230325104700434](https://user-images.githubusercontent.com/83112602/227687535-f77c2cc4-c40a-4413-8c6b-b201b9bf99bb.png)


360
![image-20230325104958465](https://user-images.githubusercontent.com/83112602/227687568-bb460140-6daf-4fde-a557-c7aa92c37681.png)


windows defender

![image-20230325104040773](https://user-images.githubusercontent.com/83112602/227687630-a221d6a8-cdf6-45db-bb69-c920831c13e7.png)


卡巴斯基

![image-20230325105224417](https://user-images.githubusercontent.com/83112602/227687680-43845183-cf5b-4488-a178-10e71036b890.png)
