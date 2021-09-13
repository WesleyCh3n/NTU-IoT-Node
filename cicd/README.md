## Upload GitHub Release from command line
### Prerequisites

- [github-release cli](https://github.com/github-release/github-release)
- [GitHub Token](https://docs.github.com/en/github/authenticating-to-github/creating-a-personal-access-token)

### How to Use

```bash
$ make V=<version> D="<release description>"

#for example
$ make V=1.1.1 D="Changelog  - foo  - bar"
```
