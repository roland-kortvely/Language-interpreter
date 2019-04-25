## Gramatika

* Declare → [declare id{"," id};] Program
* Program → Command {"}" Command}
* Command → (Read | Print | Set | While | For | If)
* Read → "read" id{"," id};
* Set → "set" id "=" Expr {"," id Expr};
* Incr → "incr" id;
* Decr → "decr" id;
* Print → "print" ("bool" Condition | [char] Expr ) {"&" ("bool" Condition | [char] Expr )};
* While → "while" (Condition) "{" Program "}"
* For → "for" (set "=" Expr {"," id Expr}; Condition; (Set | Incr | Decr); "{" Program "}"
* If → "if" (Condition) "{" Program [Program] "}" ["else" "{" Program [Program] "}"]
* Void → void POINTER () "{" Program [Program] "}"
* Exec → "exec"(POINTER);
* Condition → Expr [("<" | ">" | "<=" | ">=" | "==" | "!=") Expr]
* Expr → Mul {("+" | "-") Mul}
* Mul → Power {("*" | "/") Power}
* Power → Term ["^" Power]
* Term → VALUE | "(" Expr ")" | ID
* Exit → "exit"
