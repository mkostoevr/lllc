# Copyright 2021 Magomed Kostoev
# Copyright 2021 Nekos Team
# Published under MIT License

import sys

sys.path.append('../../')
import build

is_win32 = True if sys.platform == "win32" else False
is_linux = True if sys.platform == "linux" or sys.platform == "linux2" else False
is_osx = True if sys.platform == "darwin" else False

class TestTimeoutException(Exception):
    pass

class TestFailureException(Exception):
    pass
