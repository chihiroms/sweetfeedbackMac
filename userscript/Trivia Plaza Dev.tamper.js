// ==UserScript==
// @name       Trivia Plaza Dev
// @namespace  http://www.triviaplaza.com/qa.php
// @version    0.1
// @description  Sweetfeedback Demo!
// @match      http://www.triviaplaza.com/qa.php*
// @require  http://ajax.googleapis.com/ajax/libs/jquery/1.3.2/jquery.min.js
// @copyright  2012+, Chihiro Suga
// ==/UserScript==

jQuery(function($){
    $("#symbol").bind("DOMAttrModified", function(data){
        
        if ($(this).attr('class') == "aright") {
            alert("Congrats! Enjoy candy."); 
            $.post("http://209.129.244.24:9993/feedback?nickname=Ted&type=A", function(data) {});
        }
        
        else if ($(this).attr('class') == "awrong") {
            alert("OOps.");
        }
    
    });					  				
});
