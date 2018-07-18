;;; Directory Local Variables
;;; For more information see (info "(emacs) Directory Variables")

((c-mode
  ;;(flycheck-clang-include-path . /usr/include/python3\.6m)
  (flycheck-clang-include-path . (car (split-string (shell-command-to-string "python3-config --includes | grep -Po '(?<=-I)[^\s]+'"))))
  ))
