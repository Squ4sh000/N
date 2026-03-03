# N Language GitHub Pages 部署指南

你可以通过以下步骤将 N 语言官网部署到 `https://<你的用户名>.github.io/N/`。

## 1. 准备仓库
确保你的项目已经上传到 GitHub 仓库（例如 `https://github.com/Squ4sh000/N`）。

## 2. 启用 GitHub Pages
1.  登录 GitHub，进入你的 **N** 仓库。
2.  点击顶部的 **Settings** (设置) 选项卡。
3.  在左侧菜单中，点击 **Pages**。
4.  在 **Build and deployment** > **Source** 下，选择 **Deploy from a branch**。
5.  在 **Branch** 下，选择 `main` (或你存放代码的分支)，并将目录设置为 `/ (root)`。
    *   *提示：如果你想把官网文件单独放在一个文件夹，可以把 `website` 文件夹里的内容移动到仓库根目录，或者创建一个 `docs` 分支并将 `website` 内容放进去。*
6.  点击 **Save**。

## 3. 访问你的网站
GitHub 会自动开始构建。几分钟后，你就可以通过页面上显示的链接（通常是 `https://Squ4sh000.github.io/N/`）访问你的 N 语言官网了。

## 4. 目录说明
*   `index.html`: 网站首页。
*   `style.css`: 赛博风格的样式表。
*   `images/logo.svg`: 网站使用的官方矢量图标。

---
**提示**：建议在 `index.html` 中将下载链接修改为 GitHub Release 的真实地址，方便用户获取安装包。
