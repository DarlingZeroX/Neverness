<template name="luawindow" content="content">
<head>
	<script>
		HTML窗口 = HTML窗口 or {} --namespace

		function HTML窗口.OnWindowLoad(document)
			document:GetElementById('title').inner_rml = document.title
		end

		function HTML窗口.加载文档(name,document)
			local doc = document.context:LoadDocument('' .. name .. '.html')
			if doc then
				doc:Show()
				document:Close()
			end
			return doc
		end

		function HTML窗口.打开文档(name,document)
			local doc = document.context:LoadDocument('' .. name .. '.html')
			if doc then
				doc:Show()
			end
			return doc
		end

		function HTML窗口.ESC按下(event)
			return event.parameters['key_identifier'] == rmlui.key_identifier.ESCAPE
		end
	</script>
</head>
<body template="window">
</body>
</template>
