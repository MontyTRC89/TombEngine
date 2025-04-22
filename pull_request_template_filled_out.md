## Checklist

- [X] I have added a changelog entry to CHANGELOG.md file on the branch/fork (if it is an internal change then it is not needed) 
- [] Pull request meets the Coding Conventions standards: https://github.com/MontyTRC89/TombEngine/blob/master/CONTRIBUTING.md#coding-conventions

## Links to issue(s) this pull request concerns (if applicable)

https://github.com/TombEngine/TombEngine/issues/1584 

## Description of pull request 

filled out by contributor

two changes:

climb idle state transfers to upward climb state instead of LS_LADDER_TO_CROUCH when attempting to climb onto a flat / angled floor, so that upward climb can handle this collision

upward climb state now requires both probes to detect if there is no wall present
- LS_LADDER_TO_CROUCH should only occur if near edge of wall
- player can keep moving upwards otherwise
