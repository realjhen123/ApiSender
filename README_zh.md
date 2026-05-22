# ApiSender
一个极为轻量的API开发工具, 使用libcurl

#### language [[English]](https://github.com/realjhen123/ApiSender/blob/main/README.md) [[中文]](https://github.com/realjhen123/ApiSender/blob/main/README_zh.md)

## 如何使用
1. 下载
   
    你可以选择在 **[Github Release](https://github.com/realjhen123/ApiSender/releases/)** 中的最新版本下载
2. 运行

    Apisender将在可执行目录所在文件夹下创建Apisender文件夹, 用于存放配置文件
    > 其中config.json文件是索引和个性化相关配置, .txt文件用于存放api
3. 使用

    使用`init . .` 创建默认配置 -> 此时的工作区为 . 文件默认为Apisender.txt<br>
    或者 `init <工作区> <介绍>` 创建自定义的工作区, 保存在独立文件

    使用`sw <工作>` 或 `switch <工作>` 切换当前工作
    
    使用`l <工作区>` 或 `load <工作区>` 来加载一个工作区

    使用`rl` 或 `reload` 重新加载配置文件

    使用`set` 打开当前工作API配置文件
    |Windows|Linux|
    |-|-|
    |Notepad.exe|vim|

    你可以在打开的文件中找到你命名的工作<br>
    这里是一个例子, 是一个工作**testing**
    ```json
    {
        "testing":{
            "method": "post",
            "request":{
                "header":{
                    "Accept": "text/html,application/json",
                    "Cache-Control": "no-cache",
                    "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64)",
                    "Host": "example.com:8080"
                },
                "body":{
                    "example": "str",
                    "a":"b"
                }
            },
            "response":{
                "onJson":false,
                "type":"a.txt"
            },
            "url":"http://example.com"
        }
    }
    ```
    response的type如果为commandline,则为直接输出在命令行中,如果为文件名,则会直接输出到对应文件<br>
    response的onJson参数是可以在接受时解析Json<br>
    值得注意的, request body如果是字符串,则会原封不动的作为请求体发送<br>

    当method为get时,请求url所带的query也可用标准Json写在request body中, Apisender会自动解析<br>
    > 注意空格需要用%20替换

    使用`run` 发送所选的Api配置

    使用 `c` `cls` 或 `clear` 清屏

    使用 `ui on` 打开ui, `ui off` 关闭ui

    使用 `u` 加载上次关闭时的进度
    
## 编译
```bash
git clone https://github.com/realjhen123/apisender.git && cd Apisender
mkdir build && cd build
cmake ..
make -j12
```
### 依赖
[libcurl](https://github.com/curl/curl)
> 使用vcpkg `vcpkg install curl`或者apt `sudo apt install libcurl-dev`

## About
由于curl本身的命令行对于windwos的兼容奇奇怪怪, 写了这个软件<br>
这个软件非常的小巧, 适用于在开发时测试<br>
至于为什么不用Apifox或者postman, 内存占用实在有点多偶尔还卡<br>