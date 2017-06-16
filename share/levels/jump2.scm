;;; megajump.scm

(day)
(fog)
(set-track-name (_ "Megajump"))
(set-author "Gerard Pudenz")
(start-time (- 60 (* 15 (difficulty))))
(set-start-position 240 240)
(add-goal 189 240 #t "jump3")

(add-flag 226 241 250 #t 0.1)
(add-flag 216 240 250 #t 0.1)
(add-flag 207 241 100 #t 0.1)
(add-flag 207 240 100 #t 0.1)
(add-flag 196 241 100 #t 0.1)
(add-flag 196 240 100 #t 0.1)

