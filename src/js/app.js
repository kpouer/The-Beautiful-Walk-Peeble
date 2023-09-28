var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function getBeautifulWalk(user, raceId)
{
  var url = '';
  switch (raceId)
  {
    case 1:
      url = 'http://www.thebeautifulwalk.com/user/'+user+'/wall/alternate';
      break;
    default:
      url = 'http://www.thebeautifulwalk.com/user/'+user+'/wall';
  }
  console.log('getBeautifulWalk() ' + url);
  xhrRequest(url, 'GET', 
    function(responseText) {
      //<span>(.*)km / (.*)km vers .*(Contrisson)</span>
      var split = responseText.split('\n');
      var line1NextBound = null;
      var line2NextBound = null;
      var line1NextStep = null;
      var line2NextStep = null;
      //  var lineTotal = null;
      for (var i = 700;i<split.length;i++)
      {
        if (!line1NextBound && split[i].match(".*Prochaine frontière.*"))
        {
          line1NextBound = split[i+1];
          line2NextBound = split[i+6];
          if (raceId == 1)
            break;
        }
        if (split[i].match(".*Grande Étape.*"))
        {
          line1NextStep = split[i+1];
          line2NextStep = split[i+6];
          break;
        }
       /* if (split[i].match(".*Progression totale.*"))
        {
          lineTotal = split[i+6];
          break;
        }*/
      }
      
      //<div class="bg w100 radius4e h1v5e" data-toggle="tooltip" data-original-title="Vous avez fait 46%" data-placement="top">
      //<span>encore 252 km jusqu&#039;à Allemagne</span>
      console.log("line1NextBound="+line1NextBound);
      console.log("line2NextBound="+line2NextBound);
      var nextCityMatch    = responseText.match("<span>(.*)km / (.*)km vers (.*)</span>");
      var nextBoundMatch1  = line1NextBound.match(".*Vous avez fait (.*)%.*");
      var nextBoundMatch2  = line2NextBound.match("<span>encore (.*)\\s*km.*à (.*)</span>");
      var totalMatch       = responseText.match("<span>(.*)km / (.*)km</span>");

      if (!nextCityMatch)
        console.log('No match nextCity');
      if (!nextBoundMatch1)
        console.log('No match nextBoundMatch1 ' + line1NextBound);
      if (!nextBoundMatch2)
        console.log('No match nextBoundMatch2 ' + line2NextBound);
      if (!totalMatch)
        console.log('No match totalMatch');

      var nextStep      = null;
      var stepRemaining = 0;
      var stepPercent   = 0;

      var nextStep1 = null;
      var nextStep2 = null;
      var stepTotal = 0;
      var stepDone = 0;
      if (line1NextStep)
      {
        nextStep1 = line1NextStep.match(".*Vous avez fait (.*)%.*");
        nextStep2 = line2NextStep.match("<span>encore (.*)\\s*km.*à (.*)</span>");
        console.log("line1NextStep="+line1NextStep);
        console.log("line2NextStep="+line2NextStep);
         if (nextStep1)// si c'est peu il n'y a pas de pourcentage
        {
          nextStep = nextStep2[2];
          stepRemaining = parseInt(nextStep2[1]);
          stepPercent   = parseInt(nextStep1[1]);
          nextStep = nextStep.replace('&#039;',"'");
        }
        stepTotal = 100 * stepRemaining / (100 - stepPercent);
        stepDone  = stepTotal - stepRemaining;
        console.log("nextStep     ="+nextStep);
        console.log("stepRemaining="+stepRemaining);
        console.log("stepPercent  ="+stepPercent);
        console.log("stepTotal    ="+stepTotal);
        console.log("stepDone     ="+stepDone);
      }
      else
      {
        console.log('No match line1NextStep ' + line1NextStep);
        console.log('No match line2NextStep ' + line2NextStep);
      }

      var nextCountry      = nextBoundMatch2[2];
      var countryRemaining = parseFloat(nextBoundMatch2[1]);
      var countryPercent = 0;
      if (nextBoundMatch1)
        countryPercent = parseFloat(nextBoundMatch1[1]);
      var countryTotal     = 100 * countryRemaining / (100 - countryPercent);
      var countryDone      = countryTotal - countryRemaining;
      var totalDone        = parseFloat(totalMatch[1]);
     // var totalTotal       = parseFloat(totalMatch[2]);
      console.log("nextCountry     ="+nextCountry);
      console.log("countryRemaining="+countryRemaining);
      console.log("countryPercent  ="+countryPercent);
      console.log("countryTotal    ="+countryTotal);
      console.log("countryDone     ="+countryDone);
      

      //var txt = htmlDoc.body.children[0].children[1].children[1].children[1].children[5].children[0].children[1].children[2].innerHTML;
      // Assemble dictionary using our keys
      var dictionary = {
        //'KEY_NEXTCITY':'12345678901234567980123456789',
        'KEY_NEXTCITY':nextCityMatch[3].replace('&#039;',"'"),
        'KEY_NEXTCITYDONE':parseFloat(nextCityMatch[1]) * 100,
        'KEY_NEXTCITYTOTAL':parseFloat(nextCityMatch[2]) * 100,
        'KEY_NEXTCOUNTRY':nextCountry.replace('&#039;',"'"),
        'KEY_NEXTCOUNTRYDONE':countryDone*100,
        'KEY_NEXTCOUNTRYTOTAL':countryTotal*100,
        'KEY_NEXTSTEP':nextStep,
        'KEY_NEXTSTEPDONE':stepDone*100,
        'KEY_NEXTSTEPTOTAL':stepTotal*100,
        'KEY_TOTALDONE':totalDone*100
      };

      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log('Waze info sent to Pebble successfully!');
        },
        function(e) {
          console.log('Error sending Waze info to Pebble!');
        }
      );
    }      
  );
}

Pebble.addEventListener('ready',
  function(e) {
    console.log('PebbleKit JS ready!');
    var dictionary = { };
    
    Pebble.sendAppMessage(dictionary,
      function(e) {
        console.log('Waze info sent to Pebble successfully!');
      },
      function(e) {
        console.log('Error sending Waze info to Pebble!');
      }
    );
  }
);


Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
  //  var queryType = e.payload.KEY_QUERY;
    var walkid = e.payload.KEY_WALKID; 
    var raceId = e.payload.KEY_RACEID;
    // 'kpouer~7362688'
      getBeautifulWalk(walkid, raceId);
  }                     
);

Pebble.addEventListener('showConfiguration', function(e) 
{
  Pebble.openURL('http://pebble.kpouer.com/pebble/thebeautifulwalk/config-page.html');
});

Pebble.addEventListener('webviewclosed', function(e) 
{
  var configData = {};
  try
  {
    configData = JSON.parse(decodeURIComponent(e.response));
    console.log('Configuration page returned: ' + JSON.stringify(configData));
    var dict = {};
    dict.KEY_WALKID = configData.walkid;
  
    // Send to watchapp
    Pebble.sendAppMessage(dict, function() {
      console.log('Send successful: ' + JSON.stringify(dict));
    }, function() {
      console.log('Send failed!');
    });
  }
  catch (err)
  {
    console.log("Parse error " + err);
  }
});