local SpriteAlphaAnimator = {}

function SpriteAlphaAnimator:new(spriteRenderer)
    local obj = {}
    setmetatable(obj, self)
    self.__index = self
    
    obj.alphaDirection = 1
    obj.spriteRenderer = spriteRenderer
    obj.alphaValue = 0
    obj.isFinished = false
    return obj
end

function SpriteAlphaAnimator:Start()
    self.alphaDirection = 1
    self.alphaValue = 0
    self.isFinished = false
    self.spriteRenderer.color.w = self.alphaValue
end

function SpriteAlphaAnimator:Update(dt)
    if self.isFinished then
        return
    end
    self.alphaValue = self.alphaValue + self.alphaDirection * dt * 0.4
    if self.alphaValue >= 1 then
        self.alphaValue = 1
        self.alphaDirection = -1
    elseif self.alphaValue <= 0 then
        self.alphaValue = 0
        self.alphaDirection = 1
        self.isFinished = true
    end
    self.spriteRenderer.color.w = self.alphaValue
end

function SpriteAlphaAnimator:IsEnd()
   return self.isFinished
end

return {
    mSplash = nil,
    mGameSprite = 0,
    mLevel = 0,

    Start = function(self)
        local SpriteRenderer = self.gameObject:GetComponent('SpriteRenderer')
        self.mSplash = SpriteAlphaAnimator:new(SpriteRenderer)
        self.mSplash:Start() 
    end,

    EnterStage1 = function(self)
        self.mLevel = 1
        -- 将引擎图片设置为不可见
        local transform1 = self.gameObject:GetComponent('Transform')
        transform1.visible = false
        -- 将游戏封面设置为可见
        local transform2 = self.mGameSprite:GetComponent('Transform')
        transform2.visible = true
        -- 创建动画
        local spriteRenderer = self.mGameSprite:GetComponent('SpriteRenderer')
        self.mSplash = SpriteAlphaAnimator:new(spriteRenderer)
        self.mSplash:Start()
    end,
    Update = function(self, dt)
        if self.mLevel == 0 then
            self.mSplash:Update(dt)
            -- 如果结束进入阶段1
            if Input.GetMouseButtonDown(0) then
                self.EnterStage1(self)
            end
            if self.mSplash:IsEnd() then
                self.EnterStage1(self)
            end
        elseif self.mLevel == 1 then
            self.mSplash:Update(dt)
            -- 如果结束进入阶段2
            if Input.GetMouseButtonDown(0) then
				SceneManager:延迟加载场景("/assets/主菜单场景.vgasset")
            end
            if self.mSplash:IsEnd() then
                SceneManager:延迟加载场景("/assets/主菜单场景.vgasset")
            end
		end
    end
}