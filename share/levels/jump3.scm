;;; upstairs.scm

(day)
(set-track-name "upstairs")
(fog)
(set-author "Gerard Pudenz")
(start-time 60)
(set-start-position 220.5 221.5)
(add-goal 226 221 #t "jump4")

(if (= (difficulty) *easy*)
	(begin
	  (diamond 222.5 221.5)
	  (diamond 224.5 221.5)))

(add-flag 221 221 100 1 0.1)
(add-flag 223 221 100 1 0.1)
(add-flag 225 221 100 1 0.1)
