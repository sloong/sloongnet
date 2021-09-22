<!--
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2021-03-24 11:49:29
 * @LastEditTime: 2021-09-22 16:09:26
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/gateway/README.md
 * Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
-->

# configuration

example:
```
{
    "forwards": {
        "default": 5,
        "range": [
            {
                "begin": 100,
                "end":200,
                "forward_to": 5
            }
        ],
        "map": [
            {
                "source_function": 105,
                "map_function": 305,
                "forward_to": 5,
            }
        ]
    }
}

```

# Reference modules
All other modules
