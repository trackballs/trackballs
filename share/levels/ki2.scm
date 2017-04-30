;;; level ki2
;;; Note. This is meant to be a very easy and basic level for children

(day)
(set-track-name "At the Beach")
(set-author "Keith Winston")

(set-start-position 252.5 252.5)
(start-time 300) 

;; This is the goal
(add-goal 219 238 #f "ki3")

(add-flag 224 244 100 1 0.1) 
(add-flag 224 242 100 1 0.1) 
(add-flag 238 246 250 1 0.5)
(add-flag 242 237 100 1 0.1)
(add-flag 242 235 100 1 0.1)
(add-flag 249 241 100 1 0.1)


(define speed 0.2)
;(cond
; ((= (difficulty) *easy*) (set! speed 0.10))
; ((= (difficulty) *normal*) (set! speed 0.20))
; ((= (difficulty) *hard*) (set! speed 0.30))
;)


