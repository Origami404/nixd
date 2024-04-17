# RUN: nixd --lit-test < %s | FileCheck %s

<-- initialize(0)

```json
{
   "jsonrpc":"2.0",
   "id":0,
   "method":"initialize",
   "params":{
      "processId":123,
      "rootPath":"",
      "capabilities":{
      },
      "trace":"off"
   }
}
```


<-- textDocument/didOpen


```json
{
   "jsonrpc":"2.0",
   "method":"textDocument/didOpen",
   "params":{
      "textDocument":{
         "uri":"file:///completion.nix",
         "languageId":"nix",
         "version":1,
         "text":"with pkgs; [  ]"
      }
   }
}
```

```json
{
    "jsonrpc": "2.0",
    "id": 1,
    "method": "textDocument/completion",
    "params": {
        "textDocument": {
            "uri": "file:///completion.nix"
        },
        "position": {
            "line": 0,
            "character": 14
        },
        "context": {
            "triggerKind": 1
        }
    }
}
```

```
     CHECK:    "label": "AMB-plugins",
CHECK-NEXT:    "score": 0
CHECK-NEXT:  },
CHECK-NEXT:  {
CHECK-NEXT:    "data": "{\"Prefix\":\"\",\"Scope\":[]}",
CHECK-NEXT:    "kind": 5,
CHECK-NEXT:    "label": "ArchiSteamFarm",
CHECK-NEXT:    "score": 0
CHECK-NEXT:  },
```


```json
{"jsonrpc":"2.0","method":"exit"}
```