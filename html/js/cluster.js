/*全选功能*/
$('table th input:checkbox').on('click', function(){
    var that = this;
    $(this).closest('table').find('tr > td:first-child input:checkbox')
        .each(function(){
            this.checked = that.checked;
            $(this).closest('tr').toggleClass('selected');
        });
});
/*end*/
//表格操作
$(function(){

    //Add
    var arrAdd=[]
    $(document).on("click",".addButton",function(){
        $(".addfade").fadeIn(0)
    })
    //addArray
    $(document).on("click",".addButton_ok",function(){
        $(".addfade").fadeOut(0)
        arrAdd=[]
        var ips=document.getElementById("ips").value;
        //alert(ips)
        var url = "/addGroup?&ips="+ips;
        var xhr = new XMLHttpRequest();
        xhr.open("POST",url,true);
        xhr.onreadystatechange = function() {
            if(xhr.readyState === 4 && xhr.status === 200){
                var ret = xhr.responseText;
                if(ret === key){
                    alert("Group add sucess,please refresh the page");
                }else{
                    alert("Group add fail,please wait and try again");
                }
                window.location.reload();
            }
        };
        xhr.send();
    })

    $(document).on("click",".addButton_no",function(){
        $(".addfade").fadeOut(0)
    })

    $(document).on("click",".modal-header i",function(){
        $(".addfade").fadeOut(0)
    })


    // delete
    var del=[]
    $(document).on("click",".deleteButton",function(){
        $(".deletefade").fadeIn(0)
    })
    $(document).on("click",".deleteButton_ok",function(){
        $(".deletefade").fadeOut(0)
        arrAdd=[]
        var id=document.getElementById("Group_id").value;
        alert(id)
        var url = "/delGroup?&id="+id;
        var xhr = new XMLHttpRequest();
        xhr.open("POST",url,true);
        xhr.onreadystatechange = function() {
            if(xhr.readyState === 4 && xhr.status === 200){
                var ret = xhr.responseText;
                if(ret === key){
                    alert("Group add sucess,please refresh the page");
                }else{
                    alert("Group add fail,please wait and try again");
                }
                window.location.reload();
            }
        };
        xhr.send();
    })

    $(document).on("click",".deleteButton_no",function(){
        $(".deletefade").fadeOut(0)
    })
    $(document).on("click",".modal-header i",function(){
        $(".deletefade").fadeOut(0)
    })
    
    //view
})

    //单数行和多数行背景色设置
$(document).ready(function(){
    $("tr:odd").css("background-color","#fff");
    $("tr:even").css("background-color","#eef1f8");
});
    //改变tr的背景颜色
$(function () {
    var trEven = $("#tdata tr:even");
    trEven.mouseover(function () {
        $(this).css("background-color", "#AEF2E5");
        $(this).children("td").css("background-color", "#AEF2E5");
    }).mouseout(function () {
        $(this).css("background-color", "#FFFFFF");
        $(this).children("td").css("background-color", "#FFFFFF");
    });
    var trOdd = $("#tdata tr:odd");
    trOdd.mouseover(function () {
        $(this).css("background-color", "#AEF2E5");
        $(this).children("td").css("background-color", "#AEF2E5");
    }).mouseout(function () {
        $(this).css("background-color", "#eef1f8");
        $(this).children("td").css("background-color", "#eef1f8");
    });
});