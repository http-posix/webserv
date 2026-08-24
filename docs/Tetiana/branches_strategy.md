long-lived-branches vs short-lived branches

Trunk-Based Development — trunkbaseddevelopment.com

GitHub Flow — docs.github.com/en/get-started/using-github/github-flow
"Tip

Make a separate branch for each set of unrelated changes. This makes it easier for reviewers to give feedback. It also makes it easier for you and future collaborators to understand the changes and to revert or build on them. Additionally, if there is a delay in one set of changes, your other changes aren't also delayed."

https://www.atlassian.com/agile/software-development/branching


https://docs.aws.amazon.com/prescriptive-guidance/latest/choosing-git-branch-approach/best-practices-for-git-based-development.html

"Use short-lived branches for complex tasks – For larger or more complex tasks, use short-lived branches (also known as task branches) to work on the changes. However, make sure to keep the branch lifespan short, typically less than a day. Merge the changes back into the develop branch (Gitflow) or main branch (Trunk or GitHub Flow) as soon as possible. Smaller and more frequent merges and reviews are easier for a team to consume and process than one large merge request."

https://docs.aws.amazon.com/prescriptive-guidance/latest/choosing-git-branch-approach/git-branching-strategies.html

"Trunk – Trunk-based development is a software development practice in which all developers work on a single branch, typically called the trunk or main branch. The idea behind this approach is to keep the code base in a continuously releasable state by integrating code changes frequently and relying on automated testing and continuous integration.
GitHub Flow – GitHub Flow is a lightweight, branch-based workflow that was developed by GitHub. It is based on the idea of short-lived feature branches. When a feature is complete and ready to be deployed, the feature is merged into the main branch.
Gitflow – With a Gitflow approach, development is completed in individual feature branches. After approval, you merge feature branches into an integration branch that is usually named develop. When enough features have accumulated in the develop branch, a release branch is created to deploy the features to upper environments."