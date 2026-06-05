# ApiSender
Very Lite API Development tools power by libcurl

#### language [[English]](https://github.com/realjhen123/apisender/README.md) [[中文]](https://github.com/realjhen123/apisender/README_zh.md)

## Using
1. Download
   
    On **[Github Release](https://github.com/realjhen123/ApiSender/releases/)**
2. Run

    Apisender will make a folder so as to save configuration file.
    > The config.json file has index and personal configuration, .txt file use for save Api configuration
3. Use
   
    Using `init . .` Create a defalut config, workspace is . and file is Apisender.txt<br>
    or `init <workspace> <Introduction>` Save in other file.

    Using `sw <working> or switch <working>` to switch working.

    Using `l <workspace> or load <workspace>` to load a workspace.

    Using `set` to set up.
    |Windows|Linux|
    |-|-|
    |Notepad.exe|vim|

    Find your working<br>
    For example,working **testing**
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
                "stream":false,
                "type":"a.txt"
            },
            "url":"http://example.com"
        }
    }
    ```
    If response type = commandline,output will appear in command-line,In Case of filename,output will append on file<br>
    Response onJson can parse the json<br>
    Pay a attention, If request body is a string, It will send a Original string<br>

    When the method is get, url's query can write on request body with json, Apisender will parse<br>
    > replace the space with '%20'<br>

    On Request Body, if you using POST, you can use $(INPUT) to set when you `run` input,<br>
    or $(\<space\>`\<working\>) to replace a data,that will send a api call before main api call.<br>
    That can help you to make a Call Line from api by api.<br>

    Using `run`

    Using `c` `cls` or `clear` to clean the monitor.

    Using `ui on` to enable ui, `ui off` to disable ui.

    Using `ez on` to enable easy mode, `ez off` to disable easy mode

    > When ez mode and stream both enable,Apisender will try to parse the datapack.For AI.

    Using `u` to load last time used.
    
## Compilation
```bash
git clone https://github.com/realjhen123/apisender.git && cd Apisender
mkdir build && cd build
cmake ..
make -j12
```
### dependencies
[libcurl](https://github.com/curl/curl)
> By vcpkg `vcpkg install curl` or apt `sudo apt install libcurl-dev`

## About This Project
A better testing for develop api.<br>
There some resaon that I make is software.
1. Postcat is Using too much memory for me
2. What f**k is Apifox?
3. Curl's Command-line works strangely on Windows.