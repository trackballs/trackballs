;;; level ki3
;;; Note. This is meant to be a very easy and basic level for children

(day)
(set-track-name "Ice Skating Rink")
(set-author "Keith Winston")

(set-start-position 252.5 252.5)
(start-time 300) 

;; This is the goal
(add-goal 223 231 #t "ki4")

(add-flag 244 231 100 1 0.1) 
(add-flag 244 244 100 1 0.1) 
(add-flag 229 244 100 1 0.1)
(add-flag 234 235 100 1 0.1)
(add-flag 239 252 100 1 0.1)

(define speed 0.2)

(add-modpill 231 242 *mod-spike* 25 40)
(add-modpill 242 233 *mod-spike* 25 40)
