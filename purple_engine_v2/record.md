## RenderEngine Interface List

#### Canvas：绘图接口
```
    drawLine(x0, y0, x1, y1, paint)	绘制线条 画一条直线
    drawRect(rect, paint) 绘制矩形
    drawCircle(cx, cy, r, paint) 绘制圆形
    drawOval(rect, paint) 绘制椭圆
    drawRoundRect(rrect, paint) 绘制圆角矩形
    drawPath(path, paint) 绘制路径
    drawTextBlob(blob, x, y, paint) 绘制文本
    drawImage(image, x, y, paint) 绘制图片
    translate(dx, dy) 移动画布
    scale(sx, sy) 缩放画布
    rotate(degrees) 旋转画布
    save() 保存当前状态
    restore() 恢复上次状态
    clipRect(...) / clipPath(...) 设置裁剪区域
    saveLayer(bounds, paint) 创建临时图层

    batchBegin() //批量绘制 开始
    batchEnd() //批量绘制 结束
```

#### Paint：绘图样式控制
```
    setColor(SkColor) 设置颜色值
    setAntiAlias(true) 平滑边缘
    setStyle(kFill_Style / kStroke_Style) 填充或描边
    setStrokeWidth(width) 线条宽度
    setColorFilter(filter) 颜色效果
    setImageFilter(filter) 模糊、阴影等效果
    setBlendMode(mode) 控制合成方式
    setShader(shader) 应用渐变或图像纹理
```

#### Path：路径构建
```
    moveTo(x, y) 设置路径起点
    lineTo(x, y) 添加直线
    quadTo(cx, cy, x, y) 添加弯曲线
    cubicTo(c1x, c1y, c2x, c2y, x, y) 添加复杂曲线
    arcTo(...) 添加弧
    close() 连接回起点 关闭路径
    布尔运算
```

#### Image:图像操作
    解码图片
    绘制图像
    图像缩放
    获取像素数据
    初始化位图
    填充像素

#### Shader：渐变与贴图
    线性渐变 从一点到另一点的渐变
    径向渐变 圆形扩散渐变
    扫描渐变 环绕渐变
    图像着色器	使用图像作为填充

#### ColorFilter: 颜色与图像滤镜
    灰度滤镜
    模糊滤镜	
    阴影效果	
    多重滤镜组合

#### Font:文字渲染
    设置字体大小
    指定字体
    渲染文字



#### purple engine
    purple_render_context

    purple_resource
