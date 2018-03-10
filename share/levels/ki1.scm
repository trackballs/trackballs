;;; level kw1
;;; Note. This is meant to be a very easy and basic level for ages 4-5.
;;; Also, since I am a beginner at designing levels, I want to keep it
;;; simple at first.

(day)
(set-track-name (_ "Right Angles"))
(set-author "Keith Winston")

(set-start-position 252 252)
(start-time 300) 

;; This is the goal
(add-goal 221 237 #t "ki2")

(add-flag 250 243 100 #t 0.1) ;; first flag
(add-flag 244 242 100 #t 0.1) 
(add-flag 247 235 250 #t 0.5)
(add-flag 228 237 100 #t 0.1)

