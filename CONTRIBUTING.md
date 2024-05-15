What you will read under has been agreed by the current active team in order to keep a good flow of work and proper management. You are welcome to put suggestions in the comments in order to discuss it with the TEN team.

## General rules

**1. Developer**

- If you wish to contribute to the TEN project, you must read and follow the coding convention used by the team, available here : #294 
- Do not work directly on master nor any branch which may be developed by several members. Create your branch, and name it with a clear description of the feature/set of features you are working on. You may regularly merge/rebase master or the active development branch with yours.
- Be respectful to other developers. Disagreements may occur on code style, changes, and should be discussed directly with the person who pushed or merged the commit/branch. If you are unsure of what some parts of the code means, or why a decision was made, contact the team as a whole so they can be explained to you.
- Do not make a pull request until your branch and features are extensively tested by the QA team. You are invited to test your feature set as much as you can too.
- If you are allowed to merge PRs, do not do such until code has been properly reviewed, preferably by more than one person.
- Merge PRs using **squash and merge** mode to preserve main branch history in original state. For squash merge commit, leave list of commits in commit description (GitHub does it automatically in both desktop client and web interface).
- Do not revert active development branch to a previous commit unless you are absolutely sure there will be no regressions. Do not revert parts of code to a previous state for the same reasons. In both of those cases, the entire team needs to be aware of it and agree on the decision.
- There are **two key branches** arranged in a hierarchy: `master`, `develop`. `master` is to be used for releases, and nothing else. `develop` is based on `master` and may be used for active development. **Every PR should use it as its base**. To keep in sync, for release period they are to be merged in the following order: `develop` -> `master`.
 
**2. Tester**

- Be respectful with the developer you are testing the feature of.
- New features or bug fixes should be tested with several use cases, if not all possible use cases you can think of, of this feature. This avoids going back to it in the future because of missed test cases. 
- Pay attention to any regressions. Developer may have touched part of the code that could break other linked or unlinked features. Features or bug fixes can't be considered done if there are any regression elsewhere.

## Issues handling

**1. General**

- Preferably, write one issue for one specific bug. List of bugs should stay rare, unless they are closely related to each other.
- Follow the progress of features/fixes on the project board.
- Do not assign a dev to an issue. They will do so themselves
- Carefully choose the labels you add to the issue

**2. Developer**

- Assign yourself to an issue you want to work on available in the To do column of the project board. Add this feature to the "In progress" column. Make the team know you are working on it both by adding a comment to it and via Discord, on the strategy channel of Tomb Engine.
- Regularly update the status of the task by adding comments. If you are stuck, let other people know if they can help.
- Do not pass an issue as done unless fixes or features are fully tested and merged with active dev branch

**3. Tester**

- Give the Developer as much detail as you can about the bug you're writing for, and an easy way to reproduce it.
- Update and comment the issue with the tests you've done and your findings after fixes.

