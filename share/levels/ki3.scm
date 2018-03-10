;;; level ki3
;;; Note. This is meant to be a very easy and basic level for children

(day)
(set-track-name (_ "Ice Skating Rink"))
(set-author "Keith Winston")

(set-start-position 252 252)
(start-time 300) 

;; This is the goal
(add-goal 223 231 #t "ki4")

(add-flag 244 231 100 #t 0.1) 
(add-flag 244 244 100 #t 0.1) 
(add-flag 229 244 100 #t 0.1)
(add-flag 234 235 100 #t 0.1)
(add-flag 239 252 100 #t 0.1)

(add-modpill 230.5 241.5 *mod-spike* 25 40)
(add-modpill 241.5 232.5 *mod-spike* 25 40)
