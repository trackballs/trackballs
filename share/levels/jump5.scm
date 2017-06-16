;;; dangerousjump.scm

(day)
(set-track-name (_ "Dangerous Jump"))
(fog)
(set-author "Gerard Pudenz")
(start-time 120)
(set-start-position 239 239)

(if (< (difficulty) *hard*)
  (diamond 243 233))
(if (= (difficulty) *easy*) 
  (diamond 238 229))
(if (< (difficulty) *hard*)
  (diamond 235 225))

(add-flag 239 235 100 #t 0.1)
(add-flag 239 231 100 #t 0.1)
(add-flag 239 227 100 #t 0.1)
(add-flag 235 227 100 #t 0.1)
(add-flag 239 223 100 #t 0.1)

(add-goal 239 219 #f "")
