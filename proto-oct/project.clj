(defproject proto-oct "0.0.0-SNAPSHOT"
  :description "Proto-language for octarine, implemented as a Clojure DSL"
  :url "www.octarine-lang.se"
  :license {:name "Eclipse Public License"
            :url "http://www.eclipse.org/legal/epl-v10.html"}
  :dependencies [[org.clojure/clojure "1.4.0"]
                 [llvm-gen "0.0.1-SNAPSHOT"]]
  :main proto-oct.core)
