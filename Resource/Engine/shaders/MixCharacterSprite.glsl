VS{
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 aTexCoord;
    
    out vec2 TexCoord;
    
    void main() {
        gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
        TexCoord = aTexCoord;
    }
}
PS{
    #version 330 core
    in vec2 TexCoord;
    out vec4 FragColor;
        
    uniform sampler2D prevScene;	// 前一场景纹理
    uniform sampler2D nextScene;	// 后一场景纹理

    void main() {
        vec4 prev = texture(prevScene, TexCoord);
        vec4 next = texture(nextScene, TexCoord);

        vec3 addedColor = prev.rgb + next.rgb;
        vec3 mixedColor = prev.rgb * prev.a + next.rgb * next.a;  // 混合颜色
        float multipliedAlpha = prev.a * next.a;
        float addedAlpha = prev.a + next.a;
        float mixedAlpha = addedAlpha - multipliedAlpha;   // 混合透明度

        if(addedAlpha > 1.0)
        {
            //mixedColor = prev.rgb * 0.5 + next.rgb * 0.5;  // 混合颜色
            FragColor = vec4(next.rgb, 1.0);
            return;
        }

        addedAlpha = clamp(addedAlpha, 0.0, 1.0);

        // 防止除以零，避免NaN或错误的结果
        if (multipliedAlpha > 0.0) {
            FragColor = vec4(addedColor, addedAlpha);
        } else {
            FragColor = vec4(addedColor, mixedAlpha);
        }

        //FragColor = mix(prev, next, progress);
    }
}