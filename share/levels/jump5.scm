;;; dangerousjump.scm

(day)
(set-track-name "dangerous jump")
(fog)
(set-author "Gerard Pudenz")
(start-time 120)
(set-start-position 239.5 239.5)

(if (< (difficulty) *hard*)
	(diamond 243.5 233.5))
(if (= (difficulty) *easy*) 
	(diamond 238.5 229.5))
(if (< (difficulty) *hard*)
	(diamond 235.5 225.5))

(add-flag 239 235 100 1 0.1)
(add-flag 239 231 100 1 0.1)
(add-flag 239 227 100 1 0.1)
(add-flag 235 227 100 1 0.1)
(add-flag 239 223 100 1 0.1)

(add-goal 239 219 #f "")
