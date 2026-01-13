local 梦旅 = GalGame.引擎:创建人物('梦旅')

return function()
    梦旅:说('这是脚本数据传递2.lua')
    梦旅:说(VG.存档数据.命名空间1.变量1)

    return
end