---
layout: default
link_to_home: true
---
{% include globjects-profile.md %}

## Integration of a new Documentation

The following manual steps are necessary for each added documentation:

1. Create a subfolder with the name of the release identifier (e.g., master, v2.0)
2. Copy contents of Doxygen html folder into directory
3. Add documentation meta data to Jekyll ```_config.yml```
4. Adjust the sort order of the documentations array in the config file; the order is used for display
5. Adjust doxy-boot.js
   1. Add YAML front matter
   2. Add code to generate backlink in the ```$(document).ready``` callback
   
The resulting head of the file should look like this:

```js
---
---
$( document ).ready(function() {
    $("#projectlogo").each(function() {
        var td = $(this);
        td.html($("<a>").attr("href", "{{ site.baseurl }}/").append(td.html()));
    });
    
    // ...
});
```
