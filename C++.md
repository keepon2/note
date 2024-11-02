# 目录
lambda 表达式（也称为 lambda 函数或匿名函数） 
[capture](parameters) -> return_type {
    // function body
}
[capture]：捕获子句，定义了 lambda 表达式可以访问的外部变量。它可以是值捕获（通过复制）、引用捕获（通过引用）或两者的组合。
(parameters)：参数列表，定义了 lambda 表达式接受的参数。
-> return_type：返回类型，这是可选的。如果省略，返回类型将根据函数体自动推断。
{...}：函数体，包含 lambda 表达式的代码。


