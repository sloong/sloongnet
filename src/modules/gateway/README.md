<!--
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2021-03-24 11:49:29
 * @LastEditTime: 2021-09-23 16:34:13
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/gateway/README.md
 * Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
-->

# configuration

example:
```
{
    "forwarders": {
        "default": 5,
        "range": [
            {
                "forward_to": 5
                "begin": 100,
                "end":200,
                "convert_rule": {
                    "direction":"+/-",
                    "offset":10,
                }
            }
        ],
        "single": [
            {
                "forward_to": 5,
                "maps": [
                    "105":"305",
                ]
            }
        ]
    }
}

```

# Reference modules
All other modules
