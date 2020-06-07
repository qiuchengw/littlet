# LittleT

**注意：本项目已不再维护，代码开源，仅供技术交流。**

> `LittleT 是一个界面小清新，集任务跟踪、事件提醒、TODO 列表为一体的windows桌面端工具软件。`

- 任务跟踪
- 事件提醒
- TODO列表
- 桌面便签

## 运行截图

简介: [CSDN - blog post](https://blog.csdn.net/qiuchengw/article/details/19910779)

 ![main](https://raw.githubusercontent.com/qiuchengw/littlet/master/screenshot/main.png)
 ![main](https://raw.githubusercontent.com/qiuchengw/littlet/master/screenshot/set.jpg)

<table>
	<tr>
		<td><img src="https://raw.githubusercontent.com/qiuchengw/littlet/master/screenshot/todo.png" width="300" height="500" /></td>
		<td><img src="https://raw.githubusercontent.com/qiuchengw/littlet/master/screenshot/plan.png" width="300" height="500" /></td>
	</tr>
</table>

## 项目目录结构
```sh
├─deps     # 依赖
├─include  # 公用头文件
└─src
    ├─common  # LittleT 项目的业务逻辑文件，UI无关
    ├─LittleT # LittleT 界面相关文件
    ├─LittleTShell  # Shell 工具，支持从命令行创建任务、todo等
    ├─LittleTUpdater # 自动更新
    ├─svr       # 服务端，统计用户人数，以及用户意见反馈等
    └─UI        # UI资源，需要使用quipacker工具进行加密打包： https://github.com/qiuchengw/quipacker
```

## 编译说明

> 0. 准备windows 7或更高版本系统 + VisualStudio 2013
> 1. 签出本项目代码到`littlet`
> 2. 签出依赖的qui（https://github.com/qiuchengw/qui）项目，和`littlet`放到同一级目录下
> 3. 先编译qui，再编译`littlet`
>   3.1 使用 uipacker 打包`UI`目录到bin下（此过程是自动完成的 ，如果失败请手动编译uipacker项目，并手动打包）
> 4. 编译后的程序在 `littlet\bin`目录下
> 5. 执行exe程序，可看到效果

**说明1：**本项目的`svr`为服务端项目，用于收集用户反馈的信息，不部署并不影响客户端程序运行。

**说明2：**本项目的`LittleTShell`为客户端命令行程序，用于从命令行创建任务、TODO、定时等功能，不存在也并不影响主程序运行，功能介绍请参考`cron`项目（见下）

**说明3：**此程序内的“逻辑表达式定时器”代码质量比较差，此前已经过一轮重构，形成了一个独立的项目`cron`。但是并未合并回此项目中。重构后的项目代码开源到这儿[cron - https://github.com/qiuchengw/cron](https://github.com/qiuchengw/cron)。个人感觉这个还是蛮有用处的。能够让你在自己的项目中实现强大灵活的定时任务。支持“相对定时”和“绝对定时”。

以下是LittleTShell截图：

 ![LittleTShell](https://raw.githubusercontent.com/qiuchengw/cron/master/sc.png)


## 你可能感兴趣的项目
1. [HTMLayout](https://terrainformatica.com/a-homepage-section/htmlayout/)
2. 基于WTL和HTMLayout的GUI库：[qui - https://github.com/qiuchengw/qui](https://github.com/qiuchengw/qui)
3. **逻辑表达式定时器（很有用哦）**： [cron - https://github.com/qiuchengw/cron](https://github.com/qiuchengw/cron)


## 关于

> 作者：[qiuchengw - 漂的人](https://piaode.ren) 
> 
> 微信：qiuchengw （欢迎技术交流） 
> 
> 邮箱：qiuchengw@qq.com


## 版本发布历史

```html
更新历史：<br/>
LittleT v5.4 2015 / 7 / 19 < br / >
----------------------<br/>
增加：ctrl + shift + m 全局快捷键显示所有的便签到前台来 ，CTRL + N 创建新的便签<br/>
增加：TODO 按照重要性排序 <br/>
增加：拖动删除TODO和事件 <br/>
增加：便签上添加删除线效果 <br/>
增强：优化计划列表项的显示（之前界面会显示不全） <br/>
增强：更好的随机乱序播放图片<br/>
增强：界面显示优化（之前日期/时间地方会显示不全）<br/>
修复：在系统盘运行程序不能正确添加数据的问题（需要管理员权限）<br/>
<br/>
LittleT v5.3 2015 / 7 / 15 < br / >
----------------------<br/>
增强：支持多显示器<br/>
增强：去掉微博连接<br/>
<br/>
LittleT v5.2 2015 / 7 / 8 < br / >
----------------------<br/>
增强：便签功能Tab键缩进<br/>
增强：便签功能Ctrl + [Shift] + Tab进行便签导航<br/>
<br/>
LittleT v5.0 2015 / 6 / 11 < br / >
----------------------<br/>
增强：随机播放图片<br/>
新增：添加便签功能 <br/>
<br/>
LittleT v4.0 2015 / 3 / 18 < br / >
----------------------<br/>
增强：检测提示信息是否只有一次，如果是则需要手动关闭提示，以免错过重要的提示。
修改：提醒界面重新设计，简洁了一点（好像更难看了？！）。<br/>
修复：计划界面下拉列表不显示滚动条的问题 <br/>
<br/>
LittleT v3.8 2014 / 12 / 6 < br / >
----------------------<br/>
新增：添加到任务栏快速启动<br/>
修改：提醒界面重新设计，简介了一点（好像更难看了？！）。<br/>
修改：已用提示音保存记录，方便下次选择。
修改：多处界面微调。<br/>
<br/>
LittleT v3.6 2014/7/6<br/>
----------------------<br/>
新增：意见反馈<br/>
修改：界面微调<br/>
新增：增加了用户量统计的代码<br/>
<br/>
LittleT v3.4 2014/5/21<br/>
----------------------<br/>
新增：自动任务“每年的某天”执行功能<br/>
修改：修改为每5个小时自动检查更新<br/>
新增：我的微博连接地址<br/>

LittleT v3.3 2013/10/28<br/>
----------------------<br/>
新增：自动任务“每年的某天”执行功能
修复数个小bug

<br/>
LittleT v3.2 2013/10/28<br/>
----------------------<br/>
修复：自动任务特殊情况下提示时间不正确<br/>
修复：提示声音不播放/设置无法保存等问题<br/>
新增：开机自启动选项<br/>
增强：计划的阶段可以使用页号导航了，同时修复N个遗留问题<br/>
增强：目前ctrl+shift+h显示窗口快捷键一定会呼出窗体，无论之前在哪儿显示<br/>
增强：任务提示可以使用html代码。如 <b .red>提醒</b> 会被以红色粗体显示<br/>
<br/>
LittleT v3.1 2013/10/15<br/>
----------------------<br/>
修复：自动任务最后一次执行，界面无法正确显示下次将要执行的任务<br/>
修复：自动任务编辑后如果被执行，出现一处内存泄露<br/>
修改：日历控件界面新设计<br/>
修改：去掉侧边栏隐藏/显示的动画效果（因为xp下显示有卡顿）<br/>
修复：界面上的一些问题修复<br/>
感谢：大羊、Ooo。。。嗯 反馈bug<br/>
<br/>
LittleT v3.0 2013/6/28<br/>
----------------------<br/>
修改：“计划”界面重新设计<br/>
修复：关机任务不能被取消bug<br/>
<br/>
LittleT v2.9 2013/6/18<br/>
----------------------<br/>
<b .red>说明：不好意思，V2.8更新的update界面出错。如果您看到这个提示，请直接按下回车键，软件即可自动升级。</b><br/>
增强：记住上次退出时窗口位置，下次启动后恢复位置<br/>
修改：静默更新，不再显示更新提示<br/>
修复：通过其他软件调用产生“配置文件读取失败”问题<br/>
<br/>
LittleT v2.8 2013/6/8<br/>
----------------------<br/>
添加：程序设置对话框<br/>
修改：自动任务的时间可手动填写。<br/>
增强：最小化到系统托盘时显示气泡提示<br/>
<br/>
LittleT v2.7 2013/6/3<br/>
----------------------<br/>
修复：<b .red>实在抱歉，因时间仓促，代码维护没做到位。
导致本程序的v2.6版本不能创建自动任务。此版本仅为修复这一个低级错误。
如您在使用过程中发现bug，请及时提交给我。感激不尽。</b><br/>
<br/>
LittleT v2.6 2013/6/1<br/>
----------------------<br/>
增强：ESC键退出全屏动画<br/>
修复：header日期显示不全，感谢 Mr.我 提交bug<br/>
修复：“绝对时间执行”->“每周的某天”参数错误。感谢 李雷 提交bug<br/>
修复：全局快捷键新建todo任务，不能输入文字问题，感谢 袁飞 提交bug<br/>
<b .red>未修复bug：在屏幕上方隐藏窗口不能正确切换功能页。感谢 Mr.我 提交bug</b><br/>
<br/>
LittleT v2.5 2013/5/14<br/>
----------------------<br/>
增强："自动任务-执行程序" 可以手动输入网址了<br/>
增强：自动更新提示对话框显示详细更新内容<br/>
修改：若干界面细节调整<br/>
修改：去掉任务栏图标<br/>
修改：托盘区图标只有在程序完全隐藏的时候才会显示<br/>
修复：自动更新程序不能删除备份文件问题<br/>
修复："自动任务->绝对时间->单个日期"执行日期解析错误<br/>
<br/>
LittleT v2.4	2013/5/10<br/>
----------------------<br/>
增加：系统托盘区图标<br/>
增加：命令行方式创建自动任务<br/>
增加：靠边自动隐藏<br/>
修复：自动更新程序缺少DLL问题<br/>
修改：TODO移除“立即完成”功能<br/>
修改：界面细微调整<br/>
<br/>
LittleT v2.2	2013/4/25<br/>
----------------------<br/>
修改：“计划”中不能正确选中stage项目<br/>
增强：增加了release下面调试代码<br/>
<br/>
LittleT v2.1	2013/4/22<br/>
----------------------<br/>
修改：自动更新功能调整为每30分钟检查一次<br/>
修改：界面细节调整，准备发布<br/>
修改：自动更新功能调整为每30分钟检查一次<br/>
修改：界面细节调整，准备发布<br/>
<br/>
LittleT v2.0	2013/4/18<br/>
----------------------<br/>
增加：“计划”功能<br/>
增加：TODO立即完成倒计时<br/>
增加：程序自动更新功能<br/>
修改：界面重新设计<br/>
修复：若干个小bug<br/>
<br/>
LittleT v1.2	2013/3/18<br/>
----------------------<br/>
增强：界面加载代码重构，运行速度倍增！<br/>
修复：点击任务栏图标程序不响应消息<br/>
修复：不按下ctrl依然能使用快捷键的问题<br/>
修复：关机任务实际为重启的bug<br/>
修改：跳过自动任务时不再提示<br/>
<br/>
LittleT v1.1	2013/3/1<br/>
----------------------<br/>
新增：绝对时间任务多时间点执行<br/>
修改：重新设计了“新建自动任务”的界面，现在应该更清晰漂亮些了<br/>
<br/>
LittleT v1.0	2013/2/16<br/>
----------------------<br/>
第一个版本
```