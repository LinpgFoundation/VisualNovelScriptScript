from typing import Final, Sequence


# 角色立绘名称预处理模块
class Naming:
    # 立绘配置信息数据库
    __DATABASE: Final[dict[str, Sequence]] = {}

    @classmethod
    def get_database(cls) -> dict[str, Sequence]:
        return cls.__DATABASE

    def __init__(self, _name: str) -> None:
        _name_data: list[str] = _name.split("&")
        self.__name: str = _name_data[0]
        self.__tags: set[str] = set(_name_data[1:])

    @property
    def name(self) -> str:
        return self.__name

    @property
    def tags(self) -> set[str]:
        return self.__tags

    # 根据文件名判断是否是同一角色名下的图片
    def equal(
        self, otherNameData: "Naming" | str, must_be_the_same: bool = False
    ) -> bool:
        if isinstance(otherNameData, str):
            otherNameData = Naming(otherNameData)
        if self.__name == otherNameData.name:
            return True
        elif not must_be_the_same:
            for key in self.__DATABASE:
                if self.__name in self.__DATABASE[key]:
                    return otherNameData.name in self.__DATABASE[key]
                elif otherNameData.name in self.__DATABASE[key]:
                    return self.__name in self.__DATABASE[key]
        return False

    # 是否有tag
    def has_tag(self, _tag: str) -> bool:
        return _tag in self.__tags

    # 移除tag
    def remove_tag(self, _tag: str) -> None:
        self.__tags.remove(_tag)

    # 增加tag
    def add_tag(self, _tag: str) -> None:
        self.__tags.add(_tag)

    # 获取tag和名称结合后的数据名称
    def get_full_name(self) -> str:
        return self.__name + "".join(f"&{_tag}" for _tag in self.__tags)
