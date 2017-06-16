;;; upstairs.scm

(day)
(set-track-name (_ "Upstairs"))
(fog)
(set-author "Gerard Pudenz")
(start-time 60)
(set-start-position 220 221)
(add-goal 226 221 #t "jump4")

(if (= (difficulty) *easy*)
  (begin
    (diamond 222 221)
    (diamond 224 221)))

(add-flag 221 221 100 #t 0.1)
(add-flag 223 221 100 #t 0.1)
(add-flag 225 221 100 #t 0.1)
