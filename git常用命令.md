初始化仓库：git init  
克隆仓库：git clone命令用于从远程仓库克隆项目到本地。 git clone <repository-url>  
添加文件：git add命令将更改添加到暂存区。 git add <file-name> git add .  
提交更改：git commit命令将暂存区的更改提交到本地仓库。 git commit -m "commit message"  
查看状态：git status命令显示工作目录和暂存区的状态。 git statu  
查看差异：git diff命令比较文件的不同。 git diff  
推送更改：git push命令将本地分支的更新推送到远程仓库。 git push <remote-name> <branch-name>  
拉取更新：git pull命令从远程仓库拉取最新的更改并合并到本地分支。 git pull <remote-name> <branch-name>  
分支管理：git branch命令用于创建、列出、重命名和删除分支。 git branch <branch-name> git checkout <branch-name> git merge <branch-name> git branch -d <branch-name>  
标签管理：git tag命令用于创建、查看和删除标签。 git tag -a <tag-name> -m "tag message" git show <tag-name> git tag -d <tag-name>  

高级命令
变基：git rebase命令用于重新应用一系列提交到另一个基础上。 git rebase <branch-name>  
撤销更改：git revert命令用于撤销某次提交的更改。 git revert <commit-id>  
暂存更改：git stash命令用于暂时保存未提交的更改。 git stash git stash apply  
查找文本：git grep命令用于在项目中搜索文本。 git grep "text to search"  
垃圾回收：git gc命令用于优化本地仓库的结构。 git gc  
文件系统检查：git fsck命令用于检查文件系统的完整性。 git fsck  