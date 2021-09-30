
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
