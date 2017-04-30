;;; boxOfun

(day)
(set-track-name "Box-O'-Fun")
(set-author "Warren D Ober")
(map-is-transparent #t)
(start-time 340) 
(set-start-position 92.5 88.5)


;;1st pipe up.

(define pipe1 (pipe 111.7 127.5 0.8 114.5 127.5 1.5 0.5))
(set-wind pipe1 2.0 0.0)
(set-primary-color pipe1 0.1 0.1 0.1 1.0)
(pipe-connector 114.5 127.5 1.5 0.6)
(define pipe2 (pipe 114.5 127.5 1.5 113.5 127.5 8.6 0.5))
(set-wind pipe2 2.8 0.0)
(set-primary-color pipe2 0.1 0.1 0.1 1.0)

(add-flag 108 131 350 1 0.1)
(add-flag 125 127 350 1 0.1)
(add-flag 127 126 350 1 0.1)
(add-flag 125 125 350 1 0.1)
(add-flag 123 124 350 1 0.1)


;;1st crosspipe.


(set-primary-color (pipe 113.5 126.3 9.0 113.5 114.7 9.0 0.5) 1.0 1.0 1.0 0.4)

(diamond 117.5 116.5)

;;2nd pipe.

(define pipe3 (pipe 123.7 127.5 6.4 125.5 127.5 7.0 0.5))
(set-wind pipe3 2.0 0.0)
(set-primary-color pipe3 0.1 0.1 0.1 0.9)
(pipe-connector 125.5 127.5 7.0 0.6)
(define pipe4 (pipe 125.5 127.5 7.0 125.5 123.7 8.4 0.5))
(set-wind pipe4 2.0 0.0)
(set-primary-color pipe4 0.1 0.1 0.1 0.9)

;;2nd crosspipe.


(set-primary-color (pipe 126.5 129.3 15.0 126.5 115.5 15.0 0.5) 1.0 1.0 1.0 0.4)




(trigger 114.5 130.5 2.0 (lambda () (fog)))
(trigger 118.5 130.5 2.0 (lambda () (day)))


(diamond 110.5 130.5)




;;More crosspipes

(set-primary-color (pipe 110.5 131.7 13.5 110.5 151.3 13.5 0.5) 1.0 1.0 1.0 0.4)

(set-primary-color (pipe 110.5 153.7 13.5 110.5 173.3 13.5 0.5) 1.0 1.0 1.0 0.4)

(set-primary-color (pipe 110.5 175.7 13.5 110.5 195.3 13.5 0.5) 1.0 1.0 1.0 0.4)

(set-primary-color (pipe 110.5 197.7 13.5 110.5 219.3 13.5 0.5) 1.0 1.0 1.0 0.4)

(set-primary-color (pipe 111.7 196.5 13.5 131.3 196.5 13.5 0.5) 1.0 1.0 1.0 0.4)

(set-primary-color (pipe 109.3 174.5 13.5 89.7 174.5 13.5 0.5) 1.0 1.0 1.0 0.4)

(set-primary-color (pipe 111.7 152.5 13.5 131.3 152.5 13.5 0.5) 1.0 1.0 1.0 0.4)

(set-primary-color (pipe 132.5 175.7 13.5 132.5 195.3 13.5 0.5) 1.0 1.0 1.0 0.4)

(set-primary-color (pipe 133.7 196.5 13.5 153.3 196.5 13.5 0.5) 1.0 1.0 1.0 0.4)

(set-primary-color (pipe 154.5 195.3 13.5 154.5 175.7 13.5 0.5) 1.0 1.0 1.0 0.4)


;; Goal!
(add-goal 132 174 #f "bx2")






