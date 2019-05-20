# Processing
**1) Source that's being interpreted;**

```
five = 1 + 2 * 2
```

**2) Will create these tokens;**

Tokens | Character
--- | ---
`TOKEN_IDENTIFIER` | `five`
`TOKEN_EQUAL` | `=`
`TOKEN_NUMBER` | `1`
`TOKEN_PLUS` | `+`
`TOKEN_NUMBER` | `2`
`TOKEN_ASTERISK` | `*`
`TOKEN_NUMBER` | `2`

**3) And generating this kind of tree;**

```
      `=`
      / \
`five`   `+`
         / \
      `1`   `*`
            / \
         `2`   `2`
```

**4) By the tree we can create the bytecodes to be executed.**

Bytecode | Literal | Stack
--- | :---: | ---:
`PUSH_IDENTIFIER` | `five` | [`five`]
`PUSH_NUMBER` | `2` | [`five`, `2`]
`PUSH_NUMBER` | `2` | [`five`, `2`, `2`]
`MULTIPLY` | | [`five`, `4`]
`PUSH_NUMBER` | `1` | [`five`, `4`, `1`]
`ADD` | | [`five`, `5`]
`ASSIGN` | | []

 # Rules
 
```
varlist `=` explist
explist = exp {`,` exp}
exp = "false" | "true" | Identifier | Number | exp binop exp | unop exp
```