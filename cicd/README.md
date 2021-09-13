## Upload GitHub Release from command line
### Prerequisites

- [github-release cli](https://github.com/github-release/github-release)
- [GitHub Token](https://docs.github.com/en/github/authenticating-to-github/creating-a-personal-access-token)

### How to Use (deprecated since 2021/9/14, move to `CMakeLists`)

```bash
$ make V=<version> D="<release description>"

#for example
$ make V=1.1.1 D="Changelog  - foo  - bar"
```
